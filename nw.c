/* kfchou at altera dot com just modify it for GHRD */
/* ---------------------------------------------------  */
/*
	This Example is base on ALTERA SoC GHRD
	In GHRD, FPGA OCM is connected to H2F and offset is 0x00000000
	So the major idea is map this region from physical and try to access it 
*/
/* ---------------------------------------------------  */
/* mapleelpam at gmail dot com */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <arm_neon.h> 
#include "fifo_info.h"

#include "nw.h"
  
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
#define MAP_SIZE 0x44000
//#define MAP_SIZE 0x0100
#define MAP_MASK (MAP_SIZE - 1)

const unsigned int LOOP=10;
unsigned long mem[0xffff];
unsigned int global_mem = 0;


void* _init_mmap( const off_t target, void** map_base/*out*/, int* fd /*out*/ )
{

    void *virt_addr = NULL; 

    if((*fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) 
        FATAL;
    printf("/dev/mem opened (%x).\n",target ); fflush(stdout);

    *map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, target);
    //map_base = mmap(0, 0x10100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target);
    
    if((*map_base) == (void *) -1) 
        FATAL;

    printf("Memory mapped at address %p.\n", *map_base); fflush(stdout);

    virt_addr = (*map_base);

    return virt_addr;
}

void _deinit_mmap(  void** map_base, int* fd )
{
    if(munmap(*map_base, MAP_SIZE) == -1) FATAL;
    close(*fd);
    *fd = 0;
    *map_base = 0; 
}

uint32_t vector_add_of_n(uint32_t* ptr, uint32_t items) 
{ 
	uint32_t result,* i; 
	uint32x2_t vec64a, vec64b; 
	uint32x4_t vec128 = vdupq_n_u32(0); // clear accumulators 


	for (i=ptr; i<(ptr+items);i+=4) 
	{ 
		uint32x4_t temp128 = vld1q_u32(i); // load 4x 32 bit values 
		vec128=vaddq_u32(vec128, temp128); // add 128 bit vectors 
	} 

	vec64a = vget_low_u32(vec128); // split 128 bit vector 
	vec64b = vget_high_u32(vec128); // into 2x 64 bit vectors 

	vec64a = vadd_u32 (vec64a, vec64b); // add 64 bit vectors together 

	result = vget_lane_u32(vec64a, 0); // extract lanes and 
	result += vget_lane_u32(vec64a, 1); // add together scalars 

	return result;
}
void read_captured_mem( struct St_DPD* st_dpd  ) 
{
	int idx = 0;
	int result = 0;
	for( idx = 0 ; idx < 44 ; idx ++ )
		result ^= st_dpd->Captured[idx];
}
void write_coefficient_mem( struct St_DPD* st_dpd  ) 
{
	int idx = 0;
	for( idx = 0 ; idx < 44 ; idx ++ )
		st_dpd->Coefficient[idx] = idx + 1;
}
void do_dpd_algo( struct St_DPD* st_dpd  ) // Base on DPD function
{
	static int counter = 0;
	// Step 1 - Toggle Capture
	st_dpd->Status.capture_start = 1;
	do
	{
		/*polling*/
	}while( st_dpd->Status.capture_start == 1 );

	// Step 2 - Read Memory
	st_dpd->Status.capture_read_start = 1;
	read_captured_mem( st_dpd );	
	st_dpd->Status.capture_read_start = 0;

	// Step 3 - Do Something
	// Do something
	st_dpd->Status.dpd_calculation_start = 1;
	printf(" DPD - calculating ... %d\n",counter++);
	usleep(1000);
	st_dpd->Status.dpd_calculation_start = 0;

	// Step 4 - Write Memory 
	st_dpd->Status.coefficient_update_stat = 1;
	write_coefficient_mem( st_dpd );
	st_dpd->Status.coefficient_update_stat = 0;
}

int main()
{

    void *map_base = NULL, *virt_addr = NULL; 

    const uint32_t ALT_LWFPGA_BASE         = 0xFF200000;
    const uint32_t ALT_LWFPGA_DPD_OFFSET = 0x00040000;
    uint32_t result;

    off_t target = ALT_LWFPGA_BASE;
    int fd = 0, idx = 0;;
    struct St_DPD* st_dpd = NULL;

   int fd_c2s;
   char *myfifo = STR_PATH_C2S;

   int fd_s2c;
   char *myfifo2 = STR_PATH_S2C;

   char buf[BUFSIZ];
   char size_of_read;

    // Step 1 - Init Memory Mapping
    virt_addr = _init_mmap( target, &map_base, &fd );

    /* create the FIFO (named pipe) */
    mkfifo(myfifo, 0666);
    mkfifo(myfifo2, 0666);
    /* open, read, and display the message from the FIFO */
    fd_c2s = open(myfifo, O_RDONLY | O_NONBLOCK);
    fd_s2c = open(myfifo2, O_WRONLY);

    // Step 2 - 
    st_dpd = (struct St_DPD*)((virt_addr)+ALT_LWFPGA_DPD_OFFSET);

    // Step 3 - 
    for( ; ; ) {
	    do_dpd_algo( st_dpd );

	    // Proccess fifo commands
	    size_of_read = read(fd_c2s, buf, BUFSIZ);

	    if (size_of_read < 0 ) {
		    printf("read nothing ... continue\n");
		    usleep(10);
	    }else  if (strcmp("exit",buf)==0) {
		    printf("Server OFF.\n");
		    break;
	    } else if (strcmp("help",buf)==0) {
		    sprintf(buf,"a - command a\n"
				    "b - command b\n"
				    "c - command c\n" );
		    write(fd_s2c,buf,BUFSIZ);
	    } else if (strcmp("a",buf)==0) {
		    sprintf(buf,"execute command a\n");
		    printf("%s\n",buf);
		    write(fd_s2c,buf,BUFSIZ);
	    } else if (strcmp("b",buf)==0) {
		    sprintf(buf,"execute command b\n");
		    printf("%s\n",buf);
		    write(fd_s2c,buf,BUFSIZ);
	    } else if (strcmp("c",buf)==0) {
		    sprintf(buf,"execute command c\n");
		    printf("%s\n",buf);
		    write(fd_s2c,buf,BUFSIZ);
	    } else if (strcmp("",buf)!=0) {
		    printf("Received: %s\n", buf);
		    printf("Sending back...\n");
		    write(fd_s2c,buf,BUFSIZ);
	    } else {
	    }
	    /* clean buf from any data */
	    memset(buf, 0, sizeof(buf));

    }

    // Step 4 - uninitialize Memory Mapping
    _deinit_mmap( &map_base, &fd );


    close(fd_c2s);
    close(fd_s2c);

    unlink(myfifo);
    unlink(myfifo2);

    return 0;
}


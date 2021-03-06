#include "fifo_info.h"

int main()
{
   int fd_c2s;
   char *myfifo = STR_PATH_C2S;

   int fd_s2c;
   char *myfifo2 = STR_PATH_S2C;

   char buf[BUFSIZ];
   char size_of_read;

   /* create the FIFO (named pipe) */
   mkfifo(myfifo, 0666);
   mkfifo(myfifo2, 0666);

   /* open, read, and display the message from the FIFO */
   fd_c2s = open(myfifo, O_RDONLY | O_NONBLOCK);
   fd_s2c = open(myfifo2, O_WRONLY);

   printf("Server ON.\n");

   while (1)
   {
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

   close(fd_c2s);
   close(fd_s2c);

   unlink(myfifo);
   unlink(myfifo2);
   return 0;
}

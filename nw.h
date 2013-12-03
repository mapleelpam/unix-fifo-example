
#ifndef __NW_H__
#define __NW_H__


struct St_DPD
{
	struct {
		unsigned capture_start:1;
		unsigned unknown_1:1;
		unsigned capture_read_start:1;
		unsigned unknown_3:1;
		unsigned dpd_calculation_start:1;
		unsigned unknown_5:1;
		unsigned coefficient_update_stat:1;
		unsigned unknown_7:1;

		unsigned unknown_8_32:24;
	} Status;
	unsigned char Captured[48]; 		//0x04~0x30
	unsigned char Coefficient[48];		//0x34~0x60
}; 


#endif

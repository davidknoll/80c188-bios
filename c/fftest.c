/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for AVR              (C)ChaN, 2013    */
/*----------------------------------------------------------------------*/

#include "iofunc.h"	/* Device specific declarations */
#include "ff.h"		/* Declarations of FatFs API */
#include <string.h>

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */

int main(void)
{
	UINT br;
	char buf[128];
	if (br = f_mount(&FatFs, "", 1) != FR_OK) {		/* Give a work area to the default drive */
		outstr("Mount fail: ");
		seroutd(br);
		for (;;);
	}
	memset(buf, 0, 128);

	if (f_open(&Fil, "fatfs1.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {	/* Open a file */
		f_read(&Fil, buf, 128, &br);	/* Read data from the file */
		f_close(&Fil);								/* Close the file */
		outstr(buf);
	} else outstr("Fail 1\r\n");

	if (f_open(&Fil, "fatfs2.txt", FA_READ | FA_OPEN_EXISTING) == FR_OK) {	/* Open a file */
		f_read(&Fil, buf, 128, &br);	/* Read data from the file */
		f_close(&Fil);								/* Close the file */
		outstr(buf);
	} else outstr("Fail 2\r\n");

	outstr("Halt");
	for (;;) ;
}

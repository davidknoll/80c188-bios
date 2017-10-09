#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "bios.h"
#include "ioports.h"

/* Default interrupt vectors */
static void interrupt (* const defivt[])() = {
	int00h, int01h, int02h, int03h, int04h, int05h, int06h, int07h,	// CPU
	int08h, int09h, int0Ah, int0Bh, int0Ch, int0Dh, int0Eh, int0Fh,	// Hardware
	int10h, int11h, int12h, int13h, int14h, int15h, int16h, int17h,	// BIOS
	int18h, int19h, int1Ah, intnul, intnul, NULL, (void far *) &ddpt1440, NULL
};

static const char *imgname = "a.img";	// Image inserted in drive A:

/* Zero RAM */
static void clrmem(void)
{
	memset((void far *) 0x00000000UL, 0x00, 0x8000);
	memset((void far *) 0x08000000UL, 0x00, 0x8000);
	memset((void far *) 0x10000000UL, 0x00, 0x8000);
	memset((void far *) 0x18000000UL, 0x00, 0x8000);
	memset((void far *) 0x20000000UL, 0x00, 0x8000);
	memset((void far *) 0x28000000UL, 0x00, 0x8000);
	memset((void far *) 0x30000000UL, 0x00, 0x8000);
	memset((void far *) 0x38000000UL, 0x00, 0x8000);
	memset((void far *) 0x40000000UL, 0x00, 0x8000);
	memset((void far *) 0x48000000UL, 0x00, 0x8000);
	memset((void far *) 0x50000000UL, 0x00, 0x8000);
	memset((void far *) 0x58000000UL, 0x00, 0x8000);
	memset((void far *) 0x60000000UL, 0x00, 0x8000);
	memset((void far *) 0x68000000UL, 0x00, 0x8000);
#ifndef TESTING
	memset((void far *) 0x70000000UL, 0x00, 0x8000);	// BIOS code segment lives here during testing
	memset((void far *) 0x78000000UL, 0x00, 0x4000);	// BIOS data segment lives just above here
#endif
}

/* Initialise timer tick interrupt */
static void timinit(void)
{
	unsigned long newticks;
	unsigned int newticksh;
	unsigned char hr, min, sec;

	// Rate is 10MHz / (4 * 71 * 1934) = 18.206446539Hz
	outport(IT2_MCA, 71);		// Timer 2 as prescaler
	outport(IT2_CW, 0xC001);
	outport(IT0_MCA, 1934);		// Interrupts will come from timer 0
	outport(IT0_CW, 0xE009);
	outport(IIM_TIM, 0x0000);	// Unmask on interrupt controller, top priority

	asm {
		mov ah, 02h		// Get RTC time
		int 1Ah
		jc noclock		// If RTC not running, don't update the tick count from it
		mov hr, ch
		mov min, cl
		mov sec, dh
	}

	newticks =
		(bcdtobin(hr) * 65543UL) +	// 65543.2075404
		(bcdtobin(min) * 1093UL) +	// 1092.38679234
		(bcdtobin(sec) * 18UL);		// 18.206446539

	// Using newticks+2 in asm below was adding to the value not the pointer
	newticksh = newticks >> 16;
	asm {
		mov ah, 01h		// Set tick count
		mov cx, newticksh
		mov dx, newticks
		int 1Ah
	}
noclock:
}

/* Checksum an area of memory */
static unsigned char chksum(unsigned char *ptr, unsigned int bytes)
{
	unsigned char sum;
	while (bytes--) sum += *(ptr++);
	return sum;
}

/* Initialise option ROMs */
static void oproms(void)
{
	unsigned int opromseg = 0xC000;
	unsigned char far *curoprom;
	while (opromseg < 0xF000) {
		curoprom = (void far *) (((unsigned long) opromseg) << 16);

		if (curoprom[0] == 0x55 && curoprom[1] == 0xAA) {
			// Option ROM identifier found
			if (!chksum(curoprom, curoprom[2] * 512)) {
				// Correct checksum
				(*((void (far *)(void)) (curoprom+3)))();
				opromseg += curoprom[2] * (512/16);
				if (curoprom[2] & 3) {
					// Option ROM is not a multiple of 2KB in size
					opromseg += 2048/16;
					opromseg &= ~((2048/16)-1);
				}

			} else {
				// Incorrect checksum
				opromseg += 2048/16;
			}
		} else {
			// No option ROM identifier here
			opromseg += 2048/16;
		}
	}
}

int main()
{
	int i;
	cli();
	clrmem();

	// Init interrupt vectors
	for (i=0;i<0x20;i++) IVT[i] = defivt[i];	// CPU, hardware, BIOS
	for (i=0x20;i<0x100;i++) IVT[i] = intunh;	// Unused, shouldn't happen yet
	IVT[0x41] = (void far *) &deffdpt;			// Fixed disk parameter table
	IVT[0x4A] = intnul;							// User RTC alarm

	// Set a few things in the BIOS data area, although we don't do much with it
	*((volatile unsigned int far *) (BDA+0x10)) = 0x0041;		// Equipment word
#ifdef TESTING
	*((volatile unsigned int far *) (BDA+0x13)) = 512 - 64;		// Memory size
#else
	*((volatile unsigned int far *) (BDA+0x13)) = 512 - 16;		// Memory size
#endif
	*((volatile unsigned char far *) (BDA+0x49)) = 0x03;		// Video mode
	*((volatile unsigned int far *) (BDA+0x4A)) = 80;			// Columns
	*((volatile unsigned int far *) (BDA+0x72)) = 0x1234;		// POST flag
	*((volatile unsigned char far *) (BDA+0x75)) = 1;			// Number of HDD
	*((volatile unsigned char far *) (BDA+0x84)) = 23;			// Rows - 1

	timinit();
	probe_com();
	probe_lpt();
	// Init UART to 38400/8N1
	// Not going through Int 14h, as the UART is being used for the main console
	serinit(0x03, F_UART / (16 * 38400UL));
	// Init 8255 to all inputs in case of contention
	// Not going through Int 17h, as an 8255 is not the same thing as an LPT port
	// If adapting it to use as an LPT port, 0x82 would be more appropriate
	outportb(PPI_CTL, 0x9B);
	sti();

	// Sign-on message
	asm {
		mov ax, 0003h
		int 10h
	}
	conoutstr("\r\nDavid's 80C188 SBC BIOS\r\n");

	// Check for RTC battery low
	if ((inportb(RTC_CTLD) & 0x80) != 0x80) {
		conoutstr("RTC battery low\r\n\n");
		// Reset to 00:00, 01/01/1980
		asm {
			mov ah, 03h		// Time
			xor cx, cx
			xor dx, dx
			int 1Ah
			mov ah, 05h		// Date
			mov cx, 1980h
			mov dx, 0101h
			int 1Ah
		}

	} else {
		unsigned char hr, min, date, mth, cent, yr;
		asm {
			mov ah, 02h		// Time
			int 1Ah
			mov hr, ch
			mov min, cl
			mov ah, 04h		// Date
			int 1Ah
			mov date, dl
			mov mth, dh
			mov cent, ch
			mov yr, cl
		}

		conoutstr("Time is ");
		conouthb(hr);
		conoutb(':');
		conouthb(min);
		conoutstr(" on ");
		conouthb(date);
		conoutb('/');
		conouthb(mth);
		conoutb('/');
		conouthb(cent);
		conouthb(yr);
		conoutstr("\r\n\n");
	}

	// Init virtual disks
	conoutstr("Mounting ");
	conoutstr(imgname);
	conoutstr(" on A:... ");
	asm {
		mov ah, 60h		// Mount SD card
		mov dl, 00h
		int 13h
		jc mffail
		mov ax, 6101h	// Mount image read/write
		mov dl, 00h
		les bx, imgname
		int 13h
		jc mffail
	}
	conoutstr("OK\r\n");
	goto mfok;
mffail:
	conoutstr("failed\r\n");
mfok:

	conoutstr("Mounting hard disk image... ");
	asm {
		mov ah, 0Dh		// Alternate disk reset
		mov dl, 80h
		int 13h
		jc mhfail
	}
	conoutstr("OK\r\n");
	goto mhok;
mhfail:
	conoutstr("failed\r\n");
mhok:

	oproms();		// Init option ROMs
	asm int 19h;	// Boot
	panic("Reached end of main()");
	return 1;
}

/* Interrupt 19h
 * Boot from disk
 */
#include "bios.h"
#include "iofunc.h"

static unsigned char tryread(unsigned char drive)
{
	asm {
		xor ah, ah		// Reset disk
		mov dl, drive
		int 13h
		jc err

		mov ax, 0201h	// Read, 1 sector
		mov cx, 0001h	// Cyl 0, sector 1
		xor dx, dx		// Head 0
		mov es, dx		// To 0000:7C00
		mov bx, 7C00h
		mov dl, drive
		int 13h
		jc err
	}
	return 0;			// Carry was clear for success
err:
	return 1;			// Carry was set for error
}

static void tryboot(unsigned char drive)
{
	int retries = 4;
	while (retries--) {
		if (!tryread(drive)) {
			if (*((unsigned int far *) 0x00007DFEUL) == 0xAA55) {
				// If successful, far jump to the loaded boot sector
				asm {
					mov dl, drive	// DL = drive number
					db 0EAh			// Far JMP
					dw 7C00h, 0000h	// 0000:7C00h
				}
			}
		}
	}
}

void interrupt int19h(void)
{
	sti();
	// Try floppy, then hard disk, then ROM BASIC
	if (BDA[0x10] & 0x01) {
		outstr("Trying to boot from floppy...\r\n");
		tryboot(0x00);
	}
	if (BDA[0x75]) {
		outstr("Trying to boot from hard disk...\r\n");
		tryboot(0x80);
	}
	outstr("\r\n");
	asm int 18h;
}

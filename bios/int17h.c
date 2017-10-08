/* Interrupt 17h
 * BIOS parallel port services
 * Based partly on the anonymous 8088/V20 Turbo XT BIOS
 * This assumes a standard parallel port, which is not the same
 * thing as an 8255. It does not use 8255 modes 1 & 2, which I don't
 * believe are compatible with the printer.
 */
#include <conio.h>
#include "bios.h"

#define lptdata lptbase
#define lptstat (lptbase+1)
#define lptctl (lptbase+2)

/* Probe parallel ports and record in BIOS data area */
void probe_lpt(void)
{
	unsigned int lptio[] = { 0x3BC, 0x378, 0x278 };
	int lptcnt, lptidx;

	for (lptcnt = lptidx = 0; lptidx < 4 && lptcnt < 3; lptidx++) {
		outportb(lptio[lptidx], 0xAA);		// Test pattern
		outportb(0xC0, 0xFF);				// Noise
		if (inportb(lptio[lptidx]) == 0xAA) {
			// Found a port
			BDA[lptcnt << 1] = lptio[lptidx];
			BDA[(lptcnt << 1) + 1] = lptio[lptidx] >> 8;
			lptcnt++;
		}
	}

	BDA[0x11] &= 0x3F;
	BDA[0x11] |= (lptcnt & 0x3) << 6;
}

void interrupt int17h(struct pregs r)
{
	int i;
	unsigned int lptbase;
	sti();
	if (r.dx > 3) return;
	lptbase = *((volatile unsigned int far *) (BDA+0x08+(r.dx<<1)));
	if (!lptbase) return;
	switch (r.ax >> 8) {	// Function number in AH

	case 0x00:	// Output byte
		outportb(lptdata, r.ax);			// Data
		for (i=32000;i>0;i--) {
			if (inportb(lptstat) & 0x80) {
				outportb(lptctl, 0x0D);		// Pulse strobe pin
				outportb(lptctl, 0x0C);

				r.ax &= 0xFF;
				r.ax |= inportb(lptstat) << 8;
				r.ax &= 0xF8FF;
				r.ax ^= 0x4800;
				return;
			}
		}

		r.ax &= 0xFF;
		r.ax |= inportb(lptstat) << 8;
		r.ax |= 0x0100;						// Set timeout flag
		r.ax &= 0xF9FF;
		r.ax ^= 0x4800;
		break;

	case 0x01:	// Initialise
		outportb(lptctl, 0x08);				// Pulse init pin
		for (i=0x5DC;i>0;i--);				// Delay
		outportb(lptctl, 0x0C);

	case 0x02:	// Get status
		r.ax &= 0xFF;						// Preserve AL
		r.ax |= inportb(lptstat) << 8;		// Status in AH
		r.ax &= 0xF8FF;						// Mask out unused bits
		r.ax ^= 0x4800;						// Invert ERROR and ACK
		break;
	}
}

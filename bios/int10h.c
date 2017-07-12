/* Interrupt 10h
 * BIOS video services
 */
#include "bios.h"
#include "iofunc.h"

static const unsigned char atttosgrcol[] = {
	0,	// black
	4,	// blue
	2,	// green
	6,	// cyan
	1,	// red
	5,	// magenta
	3,	// yellow
	7	// white
};

static void doescatt(unsigned char att)
{
	static unsigned char lastatt;
	if (att == lastatt) return;			// Don't repeat unnecessarily
	lastatt = att;

	outstr("\x1B[m");					// Reset SGR
	if (att & 0x80) outstr("\x1B[5m");	// Foreground blink
	outstr("\x1B[4");					// Background colour
	seroutb('0' + atttosgrcol[(att & 0x70) >> 4]);
	seroutb('m');
	if (att & 0x08) outstr("\x1B[1m");	// Foreground bright
	outstr("\x1B[3");					// Foreground colour
	seroutb('0' + atttosgrcol[att & 0x07]);
	seroutb('m');
}

static void doescpos(int row, int col)
{
	outstr("\x1B[");
	seroutd(row + 1);
	seroutb(';');
	seroutd(col + 1);
	seroutb('H');
}

void interrupt int10h(struct pregs r)
{
	int i;
	char far *strptr;
	sti();
	switch (r.ax >> 8) {	// Function number in AH

	// Non-implemented stub functions
	case 0x04:	// Read lightpen position
		r.ax &= 0xFF;
		r.bx = 0;
	case 0x03:	// Read cursor position and size
		// Not implemented for now, although there is an escape code ESC[6n
		r.cx = r.dx = 0;
		break;
	case 0x08:	// Read character and attribute at cursor
		r.ax = 0;
		break;
	case 0x0D:	// Read graphics pixel
		r.ax &= 0xFF00;
		break;

	case 0x02:	// Set cursor position
		doescpos(r.dx >> 8, r.dx & 0xFF);
		break;

	case 0x09:	// Write character and attribute at cursor
		doescatt(r.bx);
	case 0x0A:	// Write character at cursor
		// Output character specified number of times
		for (i=0;i<r.cx;i++) seroutb(r.ax);
		break;

	case 0x0E:	// Write character in teletype mode
		// Haven't bothered to treat the colour here
		seroutb(r.ax);
		break;

	case 0x0F:	// Get current video mode
		r.ax = (80 << 8) | 0x03;	// Pretend 80 columns, mode 3
		r.bx &= 0xFF;
		break;

	case 0x13:	// Write character string
		strptr = (void far *) (((unsigned long) r.es << 16) | r.bp);
		doescpos(r.dx >> 8, r.dx & 0xFF);
		if (!(r.ax & 0x02)) doescatt(r.bx);
		if (!(r.ax & 0x01)) outstr("\x1B[s");	// Do not update cursor (save it)

		for (i=0;i<r.cx;) {
			if (r.ax & 0x02) {	// Use attributes in string
				doescatt(strptr[i+1]);
				seroutb(strptr[i]);
				i += 2;
			} else {			// Use attributes in BL
				seroutb(strptr[i++]);
			}
		}

		if (!(r.ax & 0x01)) outstr("\x1B[u");	// Do not update cursor (restore it)
		break;
	}
}

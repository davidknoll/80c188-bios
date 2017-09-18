/* Interrupt 10h
 * BIOS video services
 */
#include "bios.h"

/* Map BIOS attribute colour codes to SGR ones */
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

/* Output an SGR attribute sequence */
static void doescatt(unsigned char att)
{
	static unsigned char lastatt;
	if (att == lastatt) return;			// Don't repeat unnecessarily
	lastatt = att;

	seroutstr("\x1B[m");					// Reset SGR
	if (att & 0x80) seroutstr("\x1B[5m");	// Foreground blink
	seroutstr("\x1B[4");					// Background colour
	seroutb('0' + atttosgrcol[(att & 0x70) >> 4]);
	seroutb('m');
	if (att & 0x08) seroutstr("\x1B[1m");	// Foreground bright
	seroutstr("\x1B[3");					// Foreground colour
	seroutb('0' + atttosgrcol[att & 0x07]);
	seroutb('m');
}

/* Set the cursor position */
static void doescpos(int row, int col)
{
	seroutstr("\x1B[");
	seroutd(row + 1);
	seroutb(';');
	seroutd(col + 1);
	seroutb('H');
}

/* Resize the terminal window */
static void doescrsz(int rows, int cols)
{
	seroutstr("\x1B[8;");
	seroutd(rows);
	seroutb(';');
	seroutd(cols);
	seroutb('t');
}

/* Set scroll region */
static void doescrgn(int top, int bottom)
{
	seroutstr("\x1B[");
	seroutd(top + 1);
	seroutb(';');
	seroutd(bottom + 1);
	seroutb('r');
}

void interrupt int10h(struct pregs r)
{
	int i;
	char far *strptr;
	sti();
	switch (r.ax >> 8) {	// Function number in AH

	case 0x00:	// Set video mode
		switch (r.ax & 0x7F) {
		case 0x00:	// 40x25 grey text
		case 0x01:	// 40x25 colour text
			doescrsz(25, 40);
			BDA[0x4A] = 40;	// Columns
			break;
		case 0x02:	// 80x25 grey text
		case 0x03:	// 80x25 colour text
		case 0x07:	// 80x25 mono text
			doescrsz(25, 80);
			BDA[0x4A] = 80;
			break;
		default:
			return;
		}

		if (!(r.ax & 0x80)) {
			seroutstr("\x1B[m\f\x1B[H\x1B[2J");	// Reset SGR, clear screen
		}

		BDA[0x49] = r.ax;	// Current mode
		BDA[0x4B] = 0;		// Columns (high byte)
		BDA[0x65] |= 0x20;	// We don't support background intensity
		BDA[0x84] = 24;		// Rows - 1
		BDA[0x87] &= ~0x80;	// Bit 7 taken from the video mode
		BDA[0x87] |= (r.ax & 0x80);
		break;

	case 0x01:	// Set text mode cursor shape
		// Cursor shape can't be changed, update BDA anyway
		BDA[0x60] = r.cx & 0x1F;
		BDA[0x61] = (r.cx >> 8) & 0x1F;
		break;

	case 0x02:	// Set cursor position
		// Page number is ignored
		doescpos(r.dx >> 8, r.dx & 0xFF);
		i = 0x50 + ((r.bx & 0x0700) >> 7);
		BDA[i] = r.dx;
		BDA[i + 1] = r.dx >> 8;
		break;

	case 0x03:	// Read cursor position and size
		// As standard this function just returns values from the BDA
		// Not implemented for real, although there is an escape code ESC[6n
		i = 0x50 + ((r.bx & 0x0700) >> 7);
		r.cx = (BDA[0x61] << 8) | BDA[0x60];
		r.dx = (BDA[i + 1] << 8) | BDA[i];
		break;

	// 0x04 is for a light pen, which we don't have

	case 0x05:	// Select page
		// Just update the BDA, nothing else
		BDA[0x62] = r.ax;
		break;

	case 0x06:	// Scroll window up
	case 0x07:	// Scroll window down
		// Left/right boundaries given are ignored, scroll will be full-width
		doescrgn(r.cx >> 8, r.dx >> 8);
		doescatt(r.bx >> 8);
		// Scroll up/down, AL=00h to clear the entire window
		seroutstr("\x1B[");
		seroutd((r.ax & 0xFF) ? (r.ax & 0xFF) : ((r.dx >> 8) - (r.cx >> 8)));
		seroutb((r.ax & 0x0100) ? 'T' : 'S');
		seroutstr("\x1B[r");	// Reset scroll region now we're done
		break;

	// 0x08 read character and attribute at cursor, not implemented

	case 0x09:	// Write character and attribute at cursor
		doescatt(r.bx);		// Page number is ignored
	case 0x0A:	// Write character at cursor
		// Output character specified number of times
		seroutstr("\x1B[s");	// Do not update cursor (save it)
		for (i = 0; i < r.cx; i++) seroutb(r.ax);
		seroutstr("\x1B[u");	// Do not update cursor (restore it)
		break;

	// 0x0B set background/border/palette, nothing to do on serial
	// 0x0C & 0x0D only valid in graphics modes

	case 0x0E:	// Write character in teletype mode
		// Ignoring page number and graphics foreground pixel colour
		seroutb(r.ax);
		break;

	case 0x0F:	// Get current video mode
		r.ax = (BDA[0x4A] << 8) | BDA[0x49];
		r.bx &= 0xFF;
		r.bx |= BDA[0x62] << 8;
		break;

	// 0x10-0x12 not much to do on serial

	case 0x13:	// Write character string
		// Page number is ignored
		strptr = (void far *) (((unsigned long) r.es << 16) | r.bp);
		doescpos(r.dx >> 8, r.dx & 0xFF);
		if (!(r.ax & 0x02)) doescatt(r.bx);
		if (!(r.ax & 0x01)) seroutstr("\x1B[s");	// Do not update cursor (save it)

		for (i = 0; i < r.cx;) {
			if (r.ax & 0x02) {	// Use attributes in string
				doescatt(strptr[i + 1]);
				seroutb(strptr[i]);
				i += 2;
			} else {			// Use attributes in BL
				seroutb(strptr[i++]);
			}
		}

		if (!(r.ax & 0x01)) seroutstr("\x1B[u");	// Do not update cursor (restore it)
		break;
	}
}

/* Interrupt 16h
 * BIOS keyboard services
 */
#include "bios.h"
#include "iofunc.h"

// Reverse scancode table, so we have a scancode to return with the ASCII
static const unsigned char scancodes[] = {
	// NUL SOH   STX   ETX   EOT   ENQ   ACK   BEL    BS    HT    LF    VT    FF    CR    SO    SI
	0x03, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x0E, 0x0F, 0x24, 0x25, 0x26, 0x1C, 0x31, 0x18,
	// DLE DC1   DC2   DC3   DC4   NAK   SYN   ETB   CAN    EM   SUB   ESC    FS    GS    RS    US
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x01, 0x2B, 0x1B, 0x07, 0x0C,
	// SP    !     "     #     $     %     &     '     (     )     *     +     ,     -     .     /
	0x39, 0x02, 0x28, 0x04, 0x05, 0x06, 0x08, 0x28, 0x0A, 0x0B, 0x09, 0x0D, 0x33, 0x0C, 0x34, 0x35,
	// 0     1     2     3     4     5     6     7     8     9     :     ;     <     =     >     ?
	0x0B, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x27, 0x27, 0x33, 0x0D, 0x34, 0x35,
	// @     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O
	0x03, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	// P     Q     R     S     T     U     V     W     X     Y     Z     [     \     ]     ^     _
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x07, 0x0C,
	// `     a     b     c     d     e     f     g     h     i     j     k     l     m     n     o
	0x29, 0x1E, 0x30, 0x2E, 0x20, 0x12, 0x21, 0x22, 0x23, 0x17, 0x24, 0x25, 0x26, 0x32, 0x31, 0x18,
	// p     q     r     s     t     u     v     w     x     y     z     {     |     }     ~   DEL
	0x19, 0x10, 0x13, 0x1F, 0x14, 0x16, 0x2F, 0x11, 0x2D, 0x15, 0x2C, 0x1A, 0x2B, 0x1B, 0x29, 0x53
};

void interrupt int16h(struct pregs r)
{
	static int savec = 0;
	sti();
	switch (r.ax >> 8) {	// Function number in AH

	case 0x00:	// Read
		if (savec) {
			r.ax = savec & 0xFF;
			savec = 0;
		} else {
			r.ax = serinb();
		}
		if (r.ax < 0x80) r.ax |= scancodes[r.ax] << 8;
		break;

	case 0x01:	// Status
		if (savec) {
			// A character has already been saved below
			r.ax = savec & 0xFF;
			if (r.ax < 0x80) r.ax |= scancodes[r.ax] << 8;
			r.flags &= ~F_Z;
		} else if (serinst()) {
			// Fake returning the keystroke without removing it
			savec = 0x0100 | serinb();
			r.ax = savec & 0xFF;
			if (r.ax < 0x80) r.ax |= scancodes[r.ax] << 8;
			r.flags &= ~F_Z;
		} else {
			r.flags |= F_Z;	// Z set if no character available
		}
		break;

	// Functions 0x03 and 0x04 aren't relevant to the serial port and don't return anything

	case 0x05:	// Keyboard write
		// There's room for one byte in the "fake return" buffer from above
		r.ax &= 0xFF00;
		if (savec) {
			r.ax |= 0x01;	// Return AL=01h if buffer full
		} else {
			savec = 0x0100 | (r.cx & 0xFF);
		}
		break;

	case 0x02:	// Shift status
		// Shift key flags aren't relevant to the serial port
	case 0x09:	// Get keyboard functionality
		// Not supporting 122-key, enhanced, keyboard ID or typematic rate functions
		r.ax &= 0xFF00;
		break;
	}
}

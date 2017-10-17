/* Interrupt 14h
 * BIOS serial port services
 * Based partly on the anonymous 8088/V20 Turbo XT BIOS
 */
#include <conio.h>
#include "bios.h"
#include "ioports.h"

#define uartrbr	uartbase
#define uartthr	uartbase
#define uartier	(uartbase + 1)
#define uartiir	(uartbase + 2)
#define uartlcr	(uartbase + 3)
#define uartmcr	(uartbase + 4)
#define uartlsr	(uartbase + 5)
#define uartmsr	(uartbase + 6)
#define uartscr	(uartbase + 7)
#define uartdll	uartbase
#define uartdlm	(uartbase + 1)

/* Baud rate divisor table */
static const unsigned int baudtbl[] = {
	F_UART / (16 * 110UL),		// Available with AH=00h
	F_UART / (16 * 150UL),
	F_UART / (16 * 300UL),
	F_UART / (16 * 600UL),
	F_UART / (16 * 1200UL),
	F_UART / (16 * 2400UL),
	F_UART / (16 * 4800UL),
	F_UART / (16 * 9600UL),
	F_UART / (16 * 19200UL),	// Available with AH=04h
	F_UART / (16 * 38400UL),	// Available with ComShare
	F_UART / (16 * 57600UL),
	F_UART / (16 * 115200UL),
	0, 0, 0, 0					// Invalid
};

/* Parity mode mask table */
static const unsigned char paritbl[] = {
	0x00,	// No parity
	0x08,	// Odd parity
	0x18,	// Even parity
	0x28,	// "Stick parity odd" (mark?)
	0x38,	// "Stick parity even" (space?)
	0, 0, 0	// Invalid
};

/* Probe serial ports and record in BIOS data area */
void probe_com(void)
{
	unsigned int comio[] = { UART_BASE, 0x3F8, 0x2F8, 0x3E8, 0x2E8 };
	int comcnt, comidx;

	for (comcnt = comidx = 0; comidx < 5 && comcnt < 4; comidx++) {
		outportb(comio[comidx] + 3, 0x1A);	// LCR to 7E1
		outportb(0xC0, 0xFF);				// Noise
		if (inportb(comio[comidx] + 3) == 0x1A) {
			// Found a port
			BDA[comcnt << 1] = comio[comidx];
			BDA[(comcnt << 1) + 1] = comio[comidx] >> 8;
			comcnt++;
		}
	}

	// Number of ports found
	BDA[0x11] &= 0xF1;
	BDA[0x11] |= (comcnt & 0x7) << 1;
}

void interrupt int14h(struct pregs r)
{
	int i;
	unsigned int uartbase;
	sti();

	// Look up I/O address of that COM port number in the BDA
	if (r.dx > 3) { r.ax = 0x8000; return; }	// Error (no such port)
	uartbase = *((volatile unsigned int far *) (BDA + 0x00 + (r.dx << 1)));
	if (!uartbase) { r.ax = 0x8000; return; }

	switch (r.ax >> 8) {	// Function number in AH

	case 0x00:	// Initialise port
		outportb(uartlcr, 0x80);							// Set DLAB
		outportb(uartdll, baudtbl[r.ax >> 5]);				// Baud rate divisor
		outportb(uartdlm, baudtbl[r.ax >> 5] >> 8);
		outportb(uartlcr, r.ax & 0x1F);						// Clear DLAB, set other parameters
		outportb(uartier, 0x00);							// Disable interrupts
		r.ax = (inportb(uartlsr) << 8) | inportb(uartmsr);	// Return LSR & MSR
		break;

	case 0x01:	// Write character
		outportb(uartmcr, 0x03);							// Assert DTR, RTS
		r.ax &= 0xFF;										// Preserve character in AL
		for (i = 32000; i > 0; i--) {						// Timeout on number of attempts
			if ((inportb(uartlsr) & 0x20) && (inportb(uartmsr) & 0x30)) {	// Wait for THRE, DSR, CTS
				outportb(uartthr, r.ax);					// Output the character
				r.ax |= inportb(uartlsr) << 8;				// Return LSR in AH
				return;
			}
		}
		r.ax |= inportb(uartlsr) << 8;						// Return LSR in AH
		r.ax |= 0x8000;										// Set timeout flag
		break;

	case 0x02:	// Read character
		outportb(uartmcr, 0x01);							// Assert DTR
		r.ax &= 0xFF;										// Preserve character in AL
		for (i = 32000; i > 0; i--) {						// Timeout on number of attempts
			if ((inportb(uartlsr) & 0x01) && (inportb(uartmsr) & 0x20)) {	// Wait for DR, DSR
				r.ax = inportb(uartrbr);					// Input the character
				r.ax |= inportb(uartlsr) << 8;				// Return LSR in AH
				return;
			}
		}
		r.ax |= inportb(uartlsr) << 8;						// Return LSR in AH
		r.ax |= 0x8000;										// Set timeout flag
		break;

	case 0x03:	// Get port status
		r.ax = (inportb(uartlsr) << 8) | inportb(uartmsr);	// Return LSR & MSR
		break;

	case 0x04:	// Extended initialise (on Convertible & PS)
		outportb(uartlcr, 0x80);							// Set DLAB
		outportb(uartdll, baudtbl[r.cx & 0x0F]);			// Baud rate divisor
		outportb(uartdlm, baudtbl[r.cx & 0x0F] >> 8);
		outportb(uartlcr,
			((r.ax & 0x01) << 6) |							// Break
			((r.bx & 0x01) << 2) |							// Stop bits
			paritbl[(r.bx >> 8) & 0x07] |					// Parity
			((r.cx >> 8) & 0x03)							// Word length
		);
		outportb(uartier, 0x00);							// Disable interrupts
		r.ax = (inportb(uartlsr) << 8) | inportb(uartmsr);	// Return LSR & MSR
		break;

	case 0x05:	// Extended control (on Convertible & PS)
		switch (r.ax & 0xFF) {	// Subfunction number in AL
		case 0x00:	// Read MCR
			r.bx &= 0xFF00;
			r.bx |= inportb(uartmcr);
			break;
		case 0x01:	// Write MCR
			outportb(uartmcr, r.bx);
			break;
		}
		r.ax = (inportb(uartlsr) << 8) | inportb(uartmsr);	// Return LSR & MSR
		break;
	}
}

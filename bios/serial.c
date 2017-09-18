/* Basic onboard serial port functions
 * Should only be called directly from inside Int 10h / 16h,
 * because if these services are provided by an option ROM,
 * any console I/O from the BIOS itself needs to work with
 * the alternative device.
 */
#include <conio.h>
#include "bios.h"
#include "ioports.h"
#define RXBUFSZ 128
#define LSR_DR 1<<0
#define LSR_OE 1<<1
#define LSR_PE 1<<2
#define LSR_FE 1<<3
#define LSR_BI 1<<4
#define LSR_THRE 1<<5
#define LSR_TEMT 1<<6
#define MCR_DTR 1<<0
#define MCR_RTS 1<<1
#define MCR_OUT1 1<<2
#define MCR_OUT2 1<<3
#define MCR_LOOP 1<<4

/* Receive buffer, based on:
 * http://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular
 */
#ifndef POLLED
static volatile char rxbuf[RXBUFSZ];
static volatile int readindex = 0, writeindex = 0;

static int cbufempty(void) { return (readindex == writeindex); }

/* Producer only: updates tail index after setting the element in place */
static int cbufwr(const char c)
{
	const int wrcur = writeindex;
	const int wrnext = (wrcur + 1) % RXBUFSZ;
	if (wrnext != readindex) {
		rxbuf[wrcur] = c;
		writeindex = wrnext;
		return c;
	}
	return -1;	// full queue
}

/* Consumer only: updates head index after retrieving the element */
static int cbufrd(void)
{
	char c;
	const int rdcur = readindex;
	if (rdcur == writeindex) return -1;	// empty queue
	c = rxbuf[rdcur];
	readindex = (rdcur + 1) % RXBUFSZ;
	return c;
}
#endif

/* Basic I/O functions */
void seroutb(char c)
{
	while (!(inportb(UART_LSR) & LSR_THRE));
	outportb(UART_THR, c);
}

char serinb(void)
{
#ifdef POLLED
	while (!(inportb(UART_LSR) & LSR_DR));
	return inportb(UART_RBR);
#else
	while (cbufempty());
	return cbufrd();
#endif
}

char seroust(void) { return (inportb(UART_LSR) & LSR_THRE) ? 0xFF : 0x00; }
#ifdef POLLED
char serinst(void) { return (inportb(UART_LSR) & LSR_DR) ? 0xFF : 0x00; }
#else
char serinst(void) { return cbufempty() ? 0x00 : 0xFF; }
#endif

/* Fake the LSR for use with eg. Int 14h */
#ifndef POLLED
static volatile unsigned char lsrsave;
#endif

unsigned char serlsr(void)
{
#ifdef POLLED
	return inportb(UART_LSR);
#else
	unsigned char result = lsrsave;		// Error flags are reset when read
	lsrsave = 0;
	result |= inportb(UART_LSR) &		// Current flags
		(LSR_TEMT | LSR_THRE | LSR_BI | LSR_FE | LSR_PE | LSR_OE);
	if (!cbufempty()) result |= LSR_DR;	// Data ready?
	return result;
#endif
}

/* Initialise the UART */
void serinit(unsigned char lcr, unsigned int dl)
{
	outportb(UART_LCR, 0x80);		// Set DLAB
	outportb(UART_DLL, dl);			// Baud rate divisor
	outportb(UART_DLM, dl >> 8);
	outportb(UART_LCR, lcr);		// Clear DLAB, set other parameters

#ifdef POLLED
	outportb(UART_IER, 0x00);		// Disable interrupts
#else
	outportb(UART_IER, 0x05);		// Enable receive & line status interrupts
	outport(IIM_INT1, 0x0012);		// Unmask on interrupt controller
#endif
	inportb(UART_IIR);				// Clear pending THRE interrupt

	//outportb(UART_MCR, MCR_RTS | MCR_DTR);	// Assert RTS & DTR
	outportb(UART_MCR, (inportb(UART_MCR) & ~MCR_LOOP) | MCR_RTS);	// Loop mode off, assert RTS

#ifdef POLLED
	while (serinst()) serinb();		// Clear receive buffer
#else
	readindex = writeindex = 0;
	fakelsr();						// Clear saved error flags
#endif
}

/* Interrupt 0Dh
 * 80C186/188 external interrupt INT1 (which on my board is the UART)
 */
void interrupt int0Dh(void)
{
#ifndef POLLED
	// Fake DR will come from checking the buffer
	// THRE and TEMT will come from the real LSR at the time
	// Error flags, however, are cleared on reading the LSR
	unsigned char newlsr = inportb(UART_LSR) & (LSR_BI | LSR_FE | LSR_PE | LSR_OE);
	lsrsave |= newlsr;
	switch (inportb(UART_IIR)) {

	case 0x06:	// Receiver line status
		if (newlsr & LSR_BI) {
			// Treat like a keyboard Ctrl-Break
			BDA[0x71] = 0x01;
			asm int 1Bh;
		}
		break;

	case 0x04:	// Received data available
		// Save character in the buffer, or if it's full, set the overrun flag
		if (cbufwr(inportb(UART_RBR)) < 0) lsrsave |= LSR_OE;
		break;

	// Not using the THRE or modem status interrupts
	}
#endif
	outport(IIM_EOI, 0x000D);	// Specific EOI for INT1 input
}

/* Output a null-terminated string */
void seroutstr(const char *s) { while (*s) seroutb(*s++); }

/* Output a hex number */
void serouthn(unsigned char c)
{
	c &= 0x0F;
	if (c < 0x0A) {
		seroutb('0' + c);
	} else {
		seroutb('A' + (c - 0x0A));
	}
}

void serouthb(unsigned char c)
{
	serouthn(c >> 4);
	serouthn(c);
}

void serouthw(unsigned int i)
{
	serouthb(i >> 8);
	serouthb(i);
}

/* Input a hex number */
unsigned char serinhn(void)
{
	while (1) {
		unsigned char c = serinb();
		if (c >= '0' && c <= '9') return (c - '0');
		if (c >= 'A' && c <= 'F') return (c - 'A') + 0xA;
		if (c >= 'a' && c <= 'f') return (c - 'a') + 0xA;
	}
}

unsigned char serinhb(void)
{
	unsigned char c = serinhn() << 4;
	c |= serinhn();
	return c;
}

unsigned int serinhw(void)
{
	unsigned int i = serinhb() << 8;
	i |= serinhb();
	return i;
}

/* Output a decimal number */
void seroutd(int i)
{
	if (i < 0) {
		seroutb('-');
		i = -i;
	}

	if (i < 10) {
		serouthn(i);
	} else if (i < 100) {
		serouthb(bintobcd(i));
	} else if (i < 1000) {
		serouthn(bintobcd(i) >> 8);
		serouthb(bintobcd(i));
	} else if (i < 10000) {
		serouthw(bintobcd(i));
	} else {
		serouthn(bintobcd(i / 10000));
		serouthw(bintobcd(i));
	}
}

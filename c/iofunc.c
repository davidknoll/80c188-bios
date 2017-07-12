#include <conio.h>
#include <stdlib.h>
#include "iofunc.h"
#include "ioports.h"

/* Basic I/O to UART */
#ifndef NOSERIOB
void seroutb(char c)
{
#ifdef USEBIOS
	asm {
		mov ah, 01h
		mov al, c
		xor dx, dx
		int 14h
	}
#else
	while (!(inportb(UART_LSR) & 1<<5));	// Wait for THRE
	outportb(UART_THR, c);
#endif
}

char serinb(void)
{
#ifdef USEBIOS
	char c;
	asm {
		mov ah, 02h
		xor dx, dx
		int 14h
		mov c, al
	}
	return c;
#else
	while (!(inportb(UART_LSR) & 1<<0));	// Wait for DR
	return inportb(UART_RBR);
#endif
}

char seroust(void)
{
#ifdef USEBIOS
	char c;
	asm {
		mov ah, 03h
		xor dx, dx
		int 14h
		mov c, ah
	}
	return (c & 1<<5) ? 0xFF : 0x00;
#else
	return (inportb(UART_LSR) & 1<<5) ? 0xFF : 0x00;
#endif
}

char serinst(void)
{
#ifdef USEBIOS
	char c;
	asm {
		mov ah, 03h
		xor dx, dx
		int 14h
		mov c, ah
	}
	return (c & 1<<0) ? 0xFF : 0x00;
#else
	return (inportb(UART_LSR) & 1<<0) ? 0xFF : 0x00;
#endif
}
#endif

/* Output a null-terminated string */
void outstr(const char *s) { while (*s) seroutb(*s++); }

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

/* Convert a number to (4-digit) BCD */
int bintobcd(int i)
{
	int r = 0;
	r += (i / 1000) * 0x1000;
	i %= 1000;
	r += (i / 100) * 0x100;
	i %= 100;
	r += (i / 10) * 0x10;
	i %= 10;
	r += i;
	return r;
}

/* Convert a number from BCD to binary */
int bcdtobin(int i)
{
	int r = 0;
	r += (i / 0x1000) * 1000;
	i %= 0x1000;
	r += (i / 0x100) * 100;
	i %= 0x100;
	r += (i / 0x10) * 10;
	i %= 0x10;
	r += i;
	return r;
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

/* Input a line of text */
char *serinl(char *buf, int bufsz)
{
	char c;
	int i = 0;
	if (buf == NULL || bufsz < 2) return NULL;

	while (1) {
		c = serinb();
		if (c == 0x0D || c == 0x0A) {			// CR / LF
			buf[i] = 0x00;						// Terminating null
			outstr("\r\n");
			return buf;

		} else if (c == 0x08 || c == 0x7F) {	// BS / DEL
			if (i != 0) {
				i--;							// Back up
				outstr("\b \b");				// Erase the character on screen
			} else {
				seroutb('\a');					// Can't back up
			}

		} else if (c >= 0x20 && c <= 0x7E) {	// ASCII printable
			if (i < bufsz - 1) {
				buf[i++] = c;					// Add character to buffer
				seroutb(c);						// Echo character to screen
			} else {
				seroutb('\a');					// Won't fit in buffer
			}

		} else {								// Any other character
			seroutb('\a');
		}
	}
}

/* Get/set an interrupt vector
 * We can't use getvect and setvect from dos.h, because we don't have DOS.
 */
void setintvec(int intno, void interrupt (*handler)())
{
	void interrupt (* far *vector)();	// Pointer to pointer within IVT
	// This cast avoids a warning, and the generated code seems right
	vector = (void interrupt (* far *)()) (intno << 2);
	*vector = handler;
}

void interrupt (* getintvec(int intno))()
{
	void interrupt (* far *vector)();
	vector = (void interrupt (* far *)()) (intno << 2);
	return *vector;
}

/* Sound the beeper at the specified frequency */
void soundhz(int hz)
{
	if (hz) {
		outport(IT1_MCA, (F_CPU/8) / hz);	// Set max count A
		outport(IT1_MCB, (F_CPU/8) / hz);	// Set max count B to the same value
		outport(IT1_CW, 0xC003);			// Set control word, start the timer
	} else {
		outport(IT1_CW, 0x4003);			// Stop the timer, silence
	}
}

/* Save some faffing about with far pointers, relocatable references etc */
void pokesb(unsigned int pseg, unsigned int poff, unsigned char pdata)
{
	asm {
		mov es, pseg
		mov bx, poff
		mov al, pdata
		mov es:[bx], al
	}
}

unsigned char peeksb(unsigned int pseg, unsigned int poff)
{
	unsigned char pdata;
	asm {
		mov es, pseg
		mov bx, poff
		mov al, es:[bx]
		mov pdata, al
	}
	return pdata;
}

void pokesw(unsigned int pseg, unsigned int poff, unsigned int pdata)
{
	asm {
		mov es, pseg
		mov bx, poff
		mov ax, pdata
		mov es:[bx], ax
	}
}

unsigned int peeksw(unsigned int pseg, unsigned int poff)
{
	unsigned int pdata;
	asm {
		mov es, pseg
		mov bx, poff
		mov ax, es:[bx]
		mov pdata, ax
	}
	return pdata;
}

void panic(char *msg)
{
	outstr("\r\n ** ");
	outstr(msg);
	outstr(" ** \r\n\a");
	cli();
	while (1) hlt();
}

void far *segtofar(unsigned int pseg, unsigned int poff)
{
	void far *result;
	asm {
		mov result, word ptr poff
		mov result+2, word ptr pseg
	}
	return result;
}

/* pushcli() and popcli() are derived from those in spinlock.c in xv6,
 * a re-implementation of UNIX V6 under the MIT License, available from
 * http://pdos.csail.mit.edu/6.828/2012/xv6.html
 */
static volatile int ncli = 0;		// Current depth of pushcli()
static volatile unsigned int iflag;	// Interrupt flag before first pushcli()

void pushcli(void)
{
	unsigned int flags;
	asm {
		pushf
		pop flags
	}
	cli();
	if (ncli++ == 0) iflag = flags & 1<<9;
}

void popcli(void)
{
	unsigned int flags;
	asm {
		pushf
		pop flags
	}
	// xv6 has panics in the two bad situations below
	if (flags & 1<<9) cli();	// then it's bad- interrupts are enabled
	if (--ncli < 0) ncli = 0;	// then it's bad- more popcli's than pushcli's
	if (ncli == 0 && iflag) sti();
}

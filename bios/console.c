/* Wrappers for BIOS console I/O */
#include "bios.h"

void conoutb(const char c)
{
	asm {
		mov ah, 0eh
		mov al, c
		int 10h
	}
}

char coninb(void)
{
	char c;
	asm {
		mov ah, 00h
		int 16h
		mov c, al
	}
	return c;
}

/* Output a null-terminated string */
void conoutstr(const char *s) { while (*s) conoutb(*s++); }

/* Output a hex number */
void conouthn(unsigned char c)
{
	c &= 0x0F;
	if (c < 0x0A) {
		conoutb('0' + c);
	} else {
		conoutb('A' + (c - 0x0A));
	}
}

void conouthb(unsigned char c)
{
	conouthn(c >> 4);
	conouthn(c);
}

void conouthw(unsigned int i)
{
	conouthb(i >> 8);
	conouthb(i);
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

void panic(const char *msg)
{
	conoutstr("\r\n ** ");
	conoutstr(msg);
	conoutstr(" ** \r\n\a");
	cli();
	while (1) hlt();
}

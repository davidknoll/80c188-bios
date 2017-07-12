#include <conio.h>
#include "iofunc.h"
#include "ioports.h"

#define DO		(!(inportb(UART_MSR) & (1<<5)))					/* Test for MMC DO ('H':true, 'L':false) */
#define DI_H()	outportb(UART_MCR, inportb(UART_MCR) & ~(1<<0))	/* Set MMC DI "high" */
#define DI_L()	outportb(UART_MCR, inportb(UART_MCR) |  (1<<0))	/* Set MMC DI "low" */
#define CK_H()	outportb(UART_MCR, inportb(UART_MCR) & ~(1<<3))	/* Set MMC SCLK "high" */
#define	CK_L()	outportb(UART_MCR, inportb(UART_MCR) |  (1<<3))	/* Set MMC SCLK "low" */
#define	CS_H()	outportb(UART_MCR, inportb(UART_MCR) & ~(1<<2))	/* Set MMC CS "high" */
#define CS_L()	outportb(UART_MCR, inportb(UART_MCR) |  (1<<2))	/* Set MMC CS "low" */

char spiiob(char data)
{
	int i;
	for (i=0;i<8;i++) {
		if (data & 0x80) DI_H() else DI_L();
		CK_H();
		data <<= 1;
		if (DO) data |= 0x01;
		CK_L();
	}
	return data;
}

void spistop(void) { CS_H(); CK_L(); }
void spistart(void) { CS_L(); }

#include <conio.h>
#include "iofunc.h"
#include "ioports.h"

int main(int argc, char *argv[], char *envp[]);
void interrupt uartirq(void);

int main()
{
	/* UART IRQ test
	 * For reference, when I tried this with the RTC periodic and update
	 * interrupts it didn't work. Is this a problem with the RTC or
	 * with the transistor inverter on its interrupt output?
	 *  Install vector
	 *  Set up interrupt controller (mask, priority)
	 *  Enable UART RX interrupt
	 *  Enable interrupts (sti)
	 * It appears that inp/outp and inportb/outportb use AL (byte-wide)
	 * and inport/outport use AX (word-wide).
	 * Using my setintvec here causes an error because it generates a
	 * segment-relocatable reference, that can't be linked in a COM file.
	 */
	//setintvec(0x0D, uartirq);	// Set vector for INT1 input
	unsigned int codeseg, uioff;
	asm mov codeseg, cs;
	asm mov uioff, offset uartirq;
	pokesw(0x0000, 0x0D * 4, uioff);
	pokesw(0x0000, (0x0D * 4) + 2, codeseg);
	outport(IIM_INT1, 0x0007);	// Unmask on interrupt controller
	outportb(UART_IER, 0x01);	// Enable RX interrupt
	sti();						// Enable interrupts

	outstr("Hello, World!\r\n");
	while (1) hlt();			// Halt, waking up on interrupts
}

void interrupt uartirq(void)
{
	seroutb(serinb());			// Echo, clearing the interrupt
	outport(IIM_EOI, 0x000D);	// Specific EOI for INT1 input
}

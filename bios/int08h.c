/* Interrupt 08h
 * 80C186/188 timer 0 (system timer tick) (IRQ 0 on a PC)
 * Normally happens at 14.31818MHz / 12 / 65536 = 18.206507365Hz
 * Closest on my system 10MHz / (4 * 71 * 1934) = 18.206446539Hz
 * or 1573036.980934209 ticks per day
 */
#include <conio.h>
#include "bios.h"
#include "ioports.h"

void interrupt int08h(void)
{
	// Pointers into the BIOS data area
	volatile unsigned long far *ticks = (void far *) (BDA+0x6C);	// 0040:006C
	volatile unsigned char far *midnight = (void far *) (BDA+0x70);	// 0040:0070

	sti();
	(*ticks)++;
	if (*ticks >= 1573037UL) {	// Exceeded 24 hours
		*ticks = 0;
		(*midnight)++;
	}
	asm int 1Ch;				// Call user timer tick routine
	outport(IIM_EOI, 0x0008);	// Specific EOI for the timers
}

/* Interrupt 0Ch
 * 80C186/188 external interrupt INT0 (which on my board is the RTC)
 * NB: RTC interrupt didn't work on an early test, I think there's
 * something wrong with the transistor inverter between RTC & CPU.
 */
void interrupt int0Ch(void)
{
	sti();
	// Clear PIE (and UIE), I haven't implemented event wait yet
	outportb(RTC_CTLB, inportb(RTC_CTLB) & ~0x50);
	// Check for alarm
	if (inportb(RTC_CTLB) & inportb(RTC_CTLC) & 0x20) asm int 4Ah;
	outport(IIM_EOI, 0x000C);	// Specific EOI for INT0 input
}

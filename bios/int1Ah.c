/* Interrupt 1Ah
 * BIOS time services
 */
#include <conio.h>
#include "bios.h"
#include "ioports.h"

void interrupt int1Ah(struct pregs r)
{
	switch (r.ax >> 8) {	// Function number in AH

	case 0x00:	// Get system time
		r.dx = *((volatile unsigned int far *) (BDA+0x6C));	// Low word of tick count
		r.cx = *((volatile unsigned int far *) (BDA+0x6E));	// High word of tick count
		r.ax = BDA[0x70];									// Midnight flag
		BDA[0x70] = 0x00;
		return;

	case 0x01:	// Set system time
		*((volatile unsigned int far *) (BDA+0x6C)) = r.dx;
		*((volatile unsigned int far *) (BDA+0x6E)) = r.cx;
		BDA[0x70] = 0x00;
		return;

	case 0x02:	// Get RTC time
		outportb(RTC_CTLB, inportb(RTC_CTLB) | 0x80);	// Halt updates
		r.cx = (inportb(RTC_HR) << 8) | inportb(RTC_MIN);
		r.dx = (inportb(RTC_SEC) << 8) | (inportb(RTC_CTLB) & 0x01);
		outportb(RTC_CTLB, inportb(RTC_CTLB) & ~0x80);	// Resume updates
		break;

	case 0x03:	// Set RTC time
		outportb(RTC_CTLB, (inportb(RTC_CTLB) | 0x82) & ~0x05);				// Halt updates, set BCD & 24h, mask out DSE
		outportb(RTC_CTLA, (inportb(RTC_CTLA) & ~0x70) | 0x20);				// Enable oscillator
		outportb(RTC_HR, r.cx >> 8);
		outportb(RTC_MIN, r.cx);
		outportb(RTC_SEC, r.dx >> 8);
		outportb(RTC_CTLB, (inportb(RTC_CTLB) | (r.dx & 0x01)) & ~0x80);	// Resume updates, set DSE if required
		break;

	case 0x04:	// Get RTC date
		outportb(RTC_CTLB, inportb(RTC_CTLB) | 0x80);	// Halt updates
		// The DS12887 and MC146818 don't have a dedicated century byte.
		// The DS12C887's century byte is at 0x32.
		// The DS17887's century byte is at 0x48 in bank 1.
		r.cx = ((inportb(RTC_YR) < 0x80) ? 0x2000 : 0x1900) | inportb(RTC_YR);
		r.dx = (inportb(RTC_MTH) << 8) | inportb(RTC_DATE);
		outportb(RTC_CTLB, inportb(RTC_CTLB) & ~0x80);	// Resume updates
		break;

	case 0x05:	// Set RTC date
		outportb(RTC_CTLB, (inportb(RTC_CTLB) | 0x80) & ~0x04);	// Halt updates, set BCD
		outportb(RTC_CTLA, (inportb(RTC_CTLA) & ~0x70) | 0x20);	// Enable oscillator
		outportb(RTC_YR, r.cx);
		outportb(RTC_MTH, r.dx >> 8);
		outportb(RTC_DATE, r.dx);
		// Should probably calculate the day-of-week (RTC_DAY) here
		outportb(RTC_CTLB, inportb(RTC_CTLB) & ~0x80);			// Resume updates
		break;

	case 0x06:	// Set RTC alarm
		// Alarm time values >= C0h mean "don't care"
		outportb(RTC_HRAL, r.cx >> 8);
		outportb(RTC_MINAL, r.cx);
		outportb(RTC_SECAL, r.dx >> 8);
		outportb(RTC_CTLB, inportb(RTC_CTLB) | 0x20);	// Set AIE
		outport(IIM_INT0, 0x0013);						// Unmask
		break;

	case 0x07:	// Reset RTC alarm
		outportb(RTC_CTLB, inportb(RTC_CTLB) & ~0x20);	// Clear AIE
		break;

	//case 0x08:	// Set RTC power-on (PC Convertible)

	case 0x09:	// Read RTC alarm and status (PC Convertible)
		r.cx = (inportb(RTC_HRAL) << 8) | inportb(RTC_MINAL);
		r.dx = (inportb(RTC_SECAL) << 8) | ((inportb(RTC_CTLB) & 0x20) >> 5);
		break;

	case 0x0A:	// Read system timer day counter (some XTs only)
		r.cx = *((volatile unsigned int far *) (BDA+0xCE));	// Days since 01/01/1980
		break;

	case 0x0B:	// Set system timer day counter (some XTs only)
		*((volatile unsigned int far *) (BDA+0xCE)) = r.cx;
		break;

	//case 0x0C:	// Set RTC date/time activated power-on
	//case 0x0D:	// Reset RTC date/time activated power-on
	//case 0x0E:	// Get RTC date/time alarm status

	case 0x0F:	// Initialise RTC
		outportb(RTC_CTLA, 0x20);	// Enable oscillator, no SQW
		outportb(RTC_CTLB, 0x02);	// BCD & 24h mode, no DSE as its rules don't match the UK
		inportb(RTC_CTLC);			// Clear interrupt flags
		// DS17x87 extended control registers not supported (yet)
		break;

	//case 0x80:	// Set sound multiplexer (PCjr only)
	default:
		r.flags |= F_C;	// For these functions, indicates not supported
		return;
	}

	// Finally, check for problems with the RTC
	if (
		((inportb(RTC_CTLA) & 0x70) != 0x20) ||	// Oscillator on?
		((inportb(RTC_CTLB) & 0x06) != 0x02) ||	// BCD & 24h mode?
		((inportb(RTC_CTLD) & 0x80) != 0x80)	// Battery alive?
	) {
		r.flags |= F_C;							// Carry set to signify error
	} else {
		r.flags &= ~F_C;						// Carry clear to signify success
	}
}

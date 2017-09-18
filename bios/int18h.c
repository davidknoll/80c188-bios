/* Interrupt 18h
 * ROM BASIC / diskless boot
 */
#include <string.h>
#include "bios.h"

static void ihexrec(union farptr *loadptrptr)
{
	unsigned char bytecount, rectype, chksum;
	bytecount = serinhb();							// Byte count
	loadptrptr->segoff.off = serinhw();				// Address
	rectype = serinhb();							// Record type
	switch (rectype) {

	case 0x00:	// Data
		if (bytecount == 0) { rectype = 0xFF; break; }
		while (bytecount--) *(loadptrptr->charptr++) = serinhb();
		break;

	case 0x01:	// EOF / start address on 8-bit
		if (bytecount != 0) { rectype = 0xFF; break; }
		break;

	case 0x02:	// Segment address on 16-bit
		if (bytecount != 2) { rectype = 0xFF; break; }
		loadptrptr->segoff.seg = serinhw();			// Get new load segment
		break;

	case 0x03:	// Segmented start address on 16-bit
		if (bytecount != 4) { rectype = 0xFF; break; }
		loadptrptr->segoff.seg = serinhw();			// Get starting CS
		loadptrptr->segoff.off = serinhw();			// Get starting IP
		break;

	case 0x04:	// High word of linear address on 32-bit
		if (bytecount != 2) { rectype = 0xFF; break; }
		loadptrptr->segoff.seg = serinhw() << 12;	// Get new high word of load address (can only use its lowest nibble)
		break;

	case 0x05:	// Linear start address on 32-bit
		if (bytecount != 4) { rectype = 0xFF; break; }
		loadptrptr->segoff.seg = serinhw() << 12;	// Get high word of start address (can only use its lowest nibble)
		loadptrptr->segoff.off = serinhw();			// Get low word of start address
		break;

	}
	serinhb();										// Checksum (ignored)
	if (rectype > 0x05) {
		conoutb('?');								// Bad record
	} else if (
		rectype == 0x03 || rectype == 0x05 ||
		(rectype == 0x01 && loadptrptr->segoff.off != 0x0000)
	) {
		conoutstr("*\r\n\r\n");						// Good record with start address
		callfards(loadptrptr->funcptr);				// Call loaded code
	} else {
		conoutb('.');								// Good record
	}
}

static void srec(union farptr *loadptrptr)
{
	unsigned char bytecount, rectype, chksum;
	rectype = serinhn();							// Record type
	bytecount = serinhb();							// Byte count
	switch (rectype) {

	case 0x0:	// Block header
		bytecount -= 3;
		serinhw();									// Ignore silently
		while (bytecount--) serinhb();
		break;

	case 0x1:	// Data, 16-bit address
		bytecount -= 3;
		loadptrptr->segoff.off = serinhw();			// Address
		while (bytecount--) *(loadptrptr->charptr++) = serinhb();
		break;

	case 0x2:	// Data, 24-bit address
		bytecount -= 4;
		loadptrptr->segoff.seg = serinhb() << 12;	// Get high byte of load address (can only use its lowest nibble)
		loadptrptr->segoff.off = serinhw();			// Get low word of load address
		while (bytecount--) *(loadptrptr->charptr++) = serinhb();
		break;

	case 0x3:	// Data, 32-bit address
		bytecount -= 5;
		loadptrptr->segoff.seg = serinhw() << 12;	// Get high word of load address (can only use its lowest nibble)
		loadptrptr->segoff.off = serinhw();			// Get low word of load address
		while (bytecount--) *(loadptrptr->charptr++) = serinhb();
		break;

	case 0x5:	// Record count
		if (bytecount != 3) { rectype = 0xFF; break; }
		serinhw();									// Ignore silently
		break;

	case 0x7:	// EOF, 32-bit address
		if (bytecount != 5) { rectype = 0xFF; break; }
		loadptrptr->segoff.seg = serinhw() << 12;	// Get high word of load address (can only use its lowest nibble)
		loadptrptr->segoff.off = serinhw();			// Get low word of load address
		break;

	case 0x8:	// EOF, 24-bit address
		if (bytecount != 4) { rectype = 0xFF; break; }
		loadptrptr->segoff.seg = serinhb() << 12;	// Get high byte of start address (can only use its lowest nibble)
		loadptrptr->segoff.off = serinhw();			// Get low word of start address
		break;

	case 0x9:	// EOF, 16-bit address
		if (bytecount != 3) { rectype = 0xFF; break; }
		loadptrptr->segoff.off = serinhw();			// Address
		break;

	}
	serinhb();										// Checksum (ignored)
	if (rectype == 0x4 || rectype == 0x6 || rectype > 0x9) {
		conoutb('?');								// Bad record
	} else if (
		(rectype == 0x7 && (loadptrptr->segoff.seg | loadptrptr->segoff.off) != 0x0000) ||
		(rectype == 0x8 && (loadptrptr->segoff.seg | loadptrptr->segoff.off) != 0x0000) ||
		(rectype == 0x9 && loadptrptr->segoff.off != 0x0000)
	) {
		conoutstr("*\r\n\r\n");						// Good record with start address
		callfards(loadptrptr->funcptr);				// Call loaded code
	} else {
		conoutb('.');								// Good record
	}
}

void interrupt int18h(void)
{
	union farptr loadptr;
	sti();
	conoutstr("Hex loader for David's 80C188 SBC\r\n");
	conoutstr("Load Intel hex or Motorola S-records now, press Ctrl-B for BASIC,\r\n");
	conoutstr("or press Ctrl-R to reboot. Record checksums are currently ignored.\r\n");
	loadptr.segoff.seg = 0x0050;	// Default load segment

	while (1) {
		switch (serinb()) {
		case ':':					// Start of Intel hex record
			ihexrec(&loadptr);
			break;
		case 'S':					// Start of Motorola S-record
			srec(&loadptr);
			break;
		case 0x02:					// BASIC
			conoutstr("\r\n");
			loadptr.segoff.seg = 0x2000;
			loadptr.segoff.off = 0x0100;
			memcpy(loadptr.charptr, patb, patb_size);
			callfards(loadptr.funcptr);
			break;
		case 0x12:					// Reboot
			loadptr.segoff.seg = 0xFFFF;
			loadptr.segoff.off = 0x0000;
			callfards(loadptr.funcptr);
			break;
		}
	}
}

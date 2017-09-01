/* Interrupt 13h
 * BIOS disk services
 */
#include <stdlib.h>
#include "bios.h"
#include "ff.h"
#include "iofunc.h"
#define FDRIVES 2
#define HDSECSZ 512UL

// FatFs
static FATFS sdcard;
static FIL dskimg[FDRIVES];
static FIL hddimg;

static unsigned char dstat[FDRIVES];	// Save last status for each "drive"
static unsigned char hddstat;
static unsigned char dchange[FDRIVES];	// Change line flag

static const char *hddimgname = "c.img";

void interrupt int13h(struct pregs r)
{
	UINT bread;			// For FatFs to store number of bytes read/written
	DWORD imgoffset;	// Byte offset within image file
	UINT imgbcnt;		// Byte count to read/write
	FRESULT result;		// Temporarily save result of FatFs operation

	// Parameters
	unsigned int func = r.ax >> 8;
	unsigned int secct = r.ax & 0xFF;
	unsigned int cyl = ((r.cx & 0x00C0) << 2) | (r.cx >> 8);
	unsigned int sec = r.cx & 0x3F;
	unsigned int hd = r.dx >> 8;
	unsigned int drv = r.dx & 0xFF;
	void far *secbuf = (void far *) ((((unsigned long) r.es) << 16) | r.bx);

	struct ddpt *curddpt = &ddpt1440;	// (void far *) IVT[0x1E];
	struct fdpt *curfdpt = &deffdpt;	// (void far *) IVT[0x41];
	sti();

	// Return now if invalid drive
	if (drv >= FDRIVES && drv != 0x80) {
		r.ax &= 0xFF;
		r.ax |= 0x0100;	// Invalid command / parameter
		r.flags |= F_C;	// Carry set on error
		return;
	}

	if (drv < 0x80) {
		imgoffset = ((((cyl * 2) + hd) * curddpt->spt) + (sec - 1)) * (128UL << curddpt->bps);
		imgbcnt = secct * (128 << curddpt->bps);
	} else {
		imgoffset = ((((cyl * curfdpt->hd) + hd) * curfdpt->spt) + (sec - 1)) * HDSECSZ;
		imgbcnt = secct * HDSECSZ;
	}

	switch (func) {		// Function number in AH

	case 0x0D:	// Alternate disk reset
		if (drv < 0x80) { dstat[drv] = 0x01; break; }	// Invalid command / parameter

	case 0x00:	// Reset disk system
		if (drv < 0x80) {
			f_sync(&dskimg[drv]);
			dstat[drv] = dchange[drv] = 0x00;	// Success

		} else {
			// Close and re-open the HDD image
			f_close(&hddimg);
			result = f_open(&hddimg, hddimgname, FA_READ | FA_WRITE | FA_OPEN_EXISTING);
			if (result != FR_OK) { hddstat = 0x05; break; }	// Reset failed

			// Detect its size, using geometry from the FDPT
			curfdpt->cyl = f_size(&hddimg) / (curfdpt->hd * curfdpt->spt * HDSECSZ);
			if (curfdpt->cyl == 0 || curfdpt->cyl > 1024) {
				f_close(&hddimg);
				curfdpt->cyl = 0;
				hddstat = 0x07;	// Drive parameter activity failed
				break;
			} else {
				hddstat = 0x00;	// Success
			}
		}

	case 0x01:	// Get status of last operation
		break;

	case 0x02:	// Read sector(s)
		if (drv < 0x80) {
			if (dchange[drv]) {
				dstat[drv] = 0x06;	// Change line active
				dchange[drv] = 0;
				break;
			}
		}
		if (secct == 0 || sec == 0) {
			if (drv < 0x80) { dstat[drv] = 0x01; } else { hddstat = 0x01; }	// Invalid command / parameter
			break;
		}

		result = f_lseek((drv < 0x80) ? &dskimg[drv] : &hddimg, imgoffset);
		if (result == FR_INVALID_OBJECT) {
			if (drv < 0x80) { dstat[drv] = 0x80; } else { hddstat = 0x80; }	// Not ready
			r.ax = 0;			// AL = number of sectors read
			break;
		} else if (result != FR_OK) {
			if (drv < 0x80) { dstat[drv] = 0x40; } else { hddstat = 0x40; }	// Seek failure
			r.ax = 0;			// AL = number of sectors read
			break;
		}

		result = f_read((drv < 0x80) ? &dskimg[drv] : &hddimg, secbuf, imgbcnt, &bread);
		r.ax = bread / ((drv < 0x80) ? (128 << curddpt->bps) : HDSECSZ);
		if (result != FR_OK) {
			if (drv < 0x80) { dstat[drv] = 0x04; } else { hddstat = 0x04; }	// Sector not found
		} else if (bread != imgbcnt) {
			if (drv < 0x80) { dstat[drv] = 0x04; } else { hddstat = 0x04; }	// Sector not found
		} else {
			if (drv < 0x80) { dstat[drv] = 0x00; } else { hddstat = 0x00; }	// Success
		}
		break;

	case 0x03:	// Write sector(s)
		if (drv < 0x80) {
			if (dchange[drv]) {
				dstat[drv] = 0x06;	// Change line active
				dchange[drv] = 0;
				break;
			}
		}
		if (secct == 0 || sec == 0) {
			if (drv < 0x80) { dstat[drv] = 0x01; } else { hddstat = 0x01; }	// Invalid command / parameter
			break;
		}

		result = f_lseek((drv < 0x80) ? &dskimg[drv] : &hddimg, imgoffset);
		if (result == FR_INVALID_OBJECT) {
			if (drv < 0x80) { dstat[drv] = 0x80; } else { hddstat = 0x80; }	// Not ready
			r.ax = 0;			// AL = number of sectors written
			break;
		} else if (result != FR_OK) {
			if (drv < 0x80) { dstat[drv] = 0x40; } else { hddstat = 0x40; }	// Seek failure
			r.ax = 0;			// AL = number of sectors written
			break;
		}

#ifdef READONLY
		if (drv < 0x80) { dstat[drv] = 0x03; } else { hddstat = 0x03; }	// Write protected
		r.ax = 0;				// AL = number of sectors written
#else
		result = f_write((drv < 0x80) ? &dskimg[drv] : &hddimg, secbuf, imgbcnt, &bread);
		r.ax = bread / ((drv < 0x80) ? (128 << curddpt->bps) : HDSECSZ);
		if (result == FR_DENIED || result == FR_WRITE_PROTECTED) {
			if (drv < 0x80) { dstat[drv] = 0x03; } else { hddstat = 0x03; }	// Write protected
		} else if (result != FR_OK) {
			if (drv < 0x80) { dstat[drv] = 0x04; } else { hddstat = 0x04; }	// Sector not found
		} else if (bread != imgbcnt) {
			if (drv < 0x80) { dstat[drv] = 0x04; } else { hddstat = 0x04; }	// Sector not found
		} else {
			f_sync(&dskimg[drv]);
			if (drv < 0x80) { dstat[drv] = 0x00; } else { hddstat = 0x00; }	// Success
		}
#endif
		break;

	case 0x04:	// Verify sector(s)
		// This function doesn't verify against memory, only that the
		// sector(s) can be found and read and have the correct CRC.
		if (drv < 0x80) {
			if (dchange[drv]) {
				dstat[drv] = 0x06;	// Change line active
				dchange[drv] = 0;
				break;
			}
		}
		if (secct == 0 || sec == 0) {
			if (drv < 0x80) { dstat[drv] = 0x01; } else { hddstat = 0x01; }	// Invalid command / parameter
			break;
		}

		result = f_lseek((drv < 0x80) ? &dskimg[drv] : &hddimg, imgoffset);
		if (result == FR_INVALID_OBJECT) {
			if (drv < 0x80) { dstat[drv] = 0x80; } else { hddstat = 0x80; }	// Not ready
			r.ax = 0;			// AL = number of sectors verified
			break;
		} else if (result != FR_OK) {
			if (drv < 0x80) { dstat[drv] = 0x40; } else { hddstat = 0x40; }	// Seek failure
			r.ax = 0;			// AL = number of sectors verified
			break;
		}

		for (bread = 0; bread < imgbcnt; bread++) {
			TCHAR rdbuf;
			UINT rdcnt;
			if (f_read((drv < 0x80) ? &dskimg[drv] : &hddimg, &rdbuf, 1, &rdcnt) != FR_OK) break;
			if (rdcnt != 1) break;
		}
		if (bread != imgbcnt) {
			if (drv < 0x80) { dstat[drv] = 0x04; } else { hddstat = 0x04; }	// Sector not found
		} else {
			if (drv < 0x80) { dstat[drv] = 0x00; } else { hddstat = 0x00; }	// Success
		}
		break;

	case 0x05:	// Format track
		if (drv < 0x80) {
			if (dchange[drv]) {
				dstat[drv] = 0x06;	// Change line active
				dchange[drv] = 0;
				break;
			}
		}
		// Don't include a sector number from CL here, there isn't one
		if (drv < 0x80) {
			imgoffset = (((cyl * 2) + hd) * curddpt->spt) * (128UL << curddpt->bps);
		} else {
			imgoffset = (((cyl * curfdpt->hd) + hd) * curfdpt->spt) * HDSECSZ;
		}

		result = f_lseek((drv < 0x80) ? &dskimg[drv] : &hddimg, imgoffset);
		if (result == FR_INVALID_OBJECT) {
			if (drv < 0x80) { dstat[drv] = 0x80; } else { hddstat = 0x80; }	// Not ready
			break;
		} else if (result != FR_OK) {
			if (drv < 0x80) { dstat[drv] = 0x40; } else { hddstat = 0x40; }	// Seek failure
			break;
		} else if (drv < 0x80 && secct != curddpt->spt) {
			if (drv < 0x80) { dstat[drv] = 0x02; }	// Address mark not found
			break;
		}

#ifdef READONLY
		if (drv < 0x80) { dstat[drv] = 0x03; } else { hddstat = 0x03; }	// Write protected
#else
		// If parameters are correct, overwrite the relevant part of
		// the image file with the format filler byte
		for (bread = 0; bread < imgbcnt; bread++) {
			TCHAR wrbuf = (drv < 0x80) ? curddpt->fmtfill : 0xF6;
			UINT wrcnt;
			if (f_write((drv < 0x80) ? &dskimg[drv] : &hddimg, &wrbuf, 1, &wrcnt) != FR_OK) break;
			if (wrcnt != 1) break;
		}
		if (bread == 0 && imgbcnt != 0) {
			if (drv < 0x80) { dstat[drv] = 0x03; } else { hddstat = 0x03; }	// Write protected
		} else if (bread != imgbcnt) {
			if (drv < 0x80) { dstat[drv] = 0x04; } else { hddstat = 0x04; }	// Sector not found
		} else {
			f_sync(&dskimg[drv]);
			if (drv < 0x80) { dstat[drv] = 0x00; } else { hddstat = 0x00; }	// Success
		}
#endif
		break;

	// Functions 06-07h are for fixed disks on XT / Portable only

	case 0x08:	// Get current drive parameters
		if (drv < 0x80) {
			r.bx &= 0xFF00;
			r.bx |= curddpt->cmostype;						// BL = CMOS drive type
			r.cx = (curddpt->maxcyl << 8) | curddpt->spt;	// CH = max cyl, CL = max sec
			r.dx = (1 << 8) | FDRIVES;						// DH = max head, DL = num of drives
			r.es = ((unsigned long) curddpt) >> 16;			// ES:DI = pointer to DDPT
			r.di = ((unsigned long) curddpt) & 0xFFFF;
			dstat[drv] = 0x00;		// Success

		} else {
			r.cx = ((curfdpt->cyl - 1) << 8) | curfdpt->spt;
			r.dx = ((curfdpt->hd - 1) << 8) | 1;
			hddstat = 0x00;			// Success
		}
		break;

	case 0x09:	// Initialise fixed disk parameters
		// In this implementation we work from the FDPT at Int 41h anyway,
		// so calling this function doesn't do much
		if (drv < 0x80) { dstat[drv] = 0x01; break; }	// Invalid command / parameter
		if (
			curfdpt->cyl == 0 || curfdpt->cyl > 1024 ||
			curfdpt->hd == 0 || //curfdpt->hd > 255 ||
			curfdpt->spt == 0 || curfdpt->spt > 63
		) {
			hddstat = 0x07;	// Drive parameter activity failed
		} else {
			hddstat = 0x00;	// Success
		}
		break;

	// Functions 0A-0Bh are for diagnostics only and are not implemented

	case 0x10:	// Test for drive ready
	case 0x11:	// Recalibrate drive
		imgoffset = 0;

	case 0x0C:	// Seek to cylinder
		if (drv < 0x80) { dstat[drv] = 0x01; break; }	// Invalid command / parameter

		result = f_lseek(&hddimg, imgoffset);
		if (result == FR_INVALID_OBJECT) {
			hddstat = 0x80;		// Not ready
		} else if (result != FR_OK) {
			hddstat = 0x40;		// Seek failure
		} else {
			hddstat = 0x00;		// Success
		}
		break;

	// Function 0Dh is above
	// Functions 0E-0Fh are available on XT only / reserved for diagnostics
	// Functions 10-11h are above

	case 0x12:	// Controller RAM diagnostic
	case 0x13:	// Drive diagnostic
	case 0x14:	// Controller internal diagnostic
		// Not a lot to do here in this implementation
		if (drv < 0x80) { dstat[drv] = 0x01; break; }	// Invalid command / parameter
		r.ax = 0;
		hddstat = 0x00;			// Success
		break;

	case 0x15:	// Read DASD type
		r.ax &= 0xFF;
		if (drv < 0x80) {
			r.ax |= 0x0200;		// AH = type, floppy with changeline
			// According to RBIL and ousob, but there was a divide error on DOS startup
			//r.cx = 0;			// CX:DX = number of sectors
			//r.dx = (curddpt->maxcyl + 1) * 2 * curddpt->spt;
			dstat[drv] = 0x00;	// Success, but we're not returning 0 in AH
		} else {
			unsigned long numsecs = curfdpt->cyl * curfdpt->hd * curfdpt->spt;
			r.ax |= 0x0300;		// AH = type, fixed disk
			r.cx = numsecs >> 16;	// CX:DX = number of sectors
			r.dx = numsecs & 0xFFFF;
			hddstat = 0x00;		// Success, but we're not returning 0 in AH
		}
		r.flags &= ~F_C;
		return;

	case 0x16:	// Change line status
		if (drv == 0x80) { hddstat = 0x01; break; }	// Invalid command / parameter
		if (dchange[drv]) { dstat[drv] = 0x06; dchange[drv] = 0; break; }
		dstat[drv] = 0x00;		// Success
		break;

	case 0x17:	// Set DASD type for format
		// This function does not support 1.44MB disks
		if (drv == 0x80) { hddstat = 0x01; break; }	// Invalid command / parameter
		dstat[drv] = 0x0C;		// Unsupported track / invalid media
		break;

	case 0x18:	// Set media type for format
		if (drv == 0x80) { hddstat = 0x01; break; }	// Invalid command / parameter
		if (cyl != curddpt->maxcyl || sec != curddpt->spt) { dstat[drv] = 0x0C; break; }
		r.es = ((unsigned long) curddpt) >> 16;		// ES:DI = pointer to DDPT
		r.di = ((unsigned long) curddpt) & 0xFFFF;
		dstat[drv] = 0x00;		// Success
		break;

	// Functions 19-1Ch are for PS/2 ESDI drives

	case 0x19:	// Park heads
		if (drv < 0x80) {
			if (f_sync(&dskimg[drv]) == FR_OK) { dstat[drv] = 0x00; } else { dstat[drv] = 0x40; }
		} else {
			if (f_sync(&hddimg) == FR_OK) { hddstat = 0x00; } else { hddstat = 0x40; }
		}
		break;

	// Functions 21-25h are for PS/1 and newer PS/2 IDE drives
	// Further functions are for specific controllers and extensions
	// Functions below are specific to my FatFs-backed implementation

	case 0x60:	// Mount SD card
		f_sync(&dskimg[drv]);
		dchange[drv] = 1;
		switch (f_mount(&sdcard, "", 1)) {
		case FR_OK: dstat[drv] = 0x00; break;				// Success
		case FR_INVALID_DRIVE: dstat[drv] = 0x01; break;	// Invalid command / parameter
		case FR_DISK_ERR: dstat[drv] = 0x04; break;			// Sector not found / read error
		case FR_NOT_READY: dstat[drv] = 0x80; break;		// Not ready
		case FR_NO_FILESYSTEM: dstat[drv] = 0x0C; break;	// Unsupported track / invalid media
		default: dstat[drv] = 0x20;							// Controller failure
		}
		break;

	case 0x61:	// Mount disk image to virtual drive
		f_sync(&dskimg[drv]);
		dchange[drv] = 1;
		switch (r.ax & 0xFF) {
		case 0x00:	// Read only, must exist
			result = f_open(&dskimg[drv], secbuf, FA_READ | FA_OPEN_EXISTING);
			break;
		case 0x01:	// Read/write, must exist
			result = f_open(&dskimg[drv], secbuf, FA_READ | FA_WRITE | FA_OPEN_EXISTING);
			break;
		case 0x02:	// Read/write, create if necessary
			result = f_open(&dskimg[drv], secbuf, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
			break;
		case 0x03:	// Read/write, must create
			result = f_open(&dskimg[drv], secbuf, FA_READ | FA_WRITE | FA_CREATE_NEW);
			break;
		default:
			result = FR_INVALID_DRIVE;
		}

		switch (result) {
		case FR_OK:
			// Check for / expand to correct image size
			imgoffset = (curddpt->maxcyl + 1) * 2 * curddpt->spt * (128UL << curddpt->bps);
			f_lseek(&dskimg[drv], imgoffset);
			if (f_tell(&dskimg[drv]) != imgoffset) {
				f_close(&dskimg[drv]);
				dstat[drv] = 0x0C;	// Unsupported track / invalid media
			} else {
				f_sync(&dskimg[drv]);
				dstat[drv] = 0x00;	// Success
			}
			break;

		case FR_INVALID_DRIVE: dstat[drv] = 0x01; break;	// Invalid command / parameter
		case FR_DISK_ERR: dstat[drv] = 0x04; break;			// Sector not found / read error
		case FR_INT_ERR: dstat[drv] = 0x20; break;			// Controller failure
		case FR_DENIED:
		case FR_WRITE_PROTECTED: dstat[drv] = 0x03; break;	// Write protected
		default: dstat[drv] = 0x80;							// Not ready
		}
		break;

	case 0x62:	// Unmount disk image from virtual drive
		dchange[drv] = 1;
		if (f_close(&dskimg[drv]) != FR_OK) { dstat[drv] = 0x04; break; }
		dstat[drv] = 0x00;
		break;

	default:
		if (drv < 0x80) { dstat[drv] = 0x01; } else { hddstat = 0x01; }	// Invalid command / parameter
	}

	// Return with status in AH and carry set on error
	BDA[0x41] = dstat[drv];
	BDA[0x74] = hddstat;
	r.ax &= 0xFF;
	if (drv < 0x80) {
		r.ax |= dstat[drv] << 8;
		if (dstat[drv]) { r.flags |= F_C; } else { r.flags &= ~F_C; }
	} else {
		r.ax |= hddstat << 8;
		if (hddstat) { r.flags |= F_C; } else { r.flags &= ~F_C; }
	}
}

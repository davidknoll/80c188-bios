/* Interrupt 15h
 * BIOS cassette / misc services
 */
#include <dos.h>
#include "bios.h"
#include "ff.h"
#include "iofunc.h"
#define FILES 4

static FIL fatfsfile[FILES];

// Table returned by AH=C0h (Get Configuration)
static const unsigned char cfgtbl[] = {
	0x08, 0x00,						// Number of bytes following
	0xFE, 0x00,						// Model (here XT), submodel
	0x00,							// BIOS revision
	0x20, 0x44, 0x00, 0x48, 0x01	// Features
};

void interrupt int15h(struct pregs r)
{
	UINT bread;			// For FatFs to store number of bytes read/written
	FRESULT result;		// Temporarily save result of FatFs operation

	// Parameters
	unsigned char func = r.ax >> 8;
	unsigned char fileno = r.ax & 0xFF;
	unsigned long offset = (((unsigned long) r.cx) << 16) | r.dx;
	unsigned int bcnt = r.dx;
	void far *rwbuf = (void far *) ((((unsigned long) r.es) << 16) | r.bx);

	sti();
	switch (func) {		// Function number in AH

	// Functions 00-03h are for the cassette system on the original PC
	// Others are reserved and/or used by various BIOSes in various ways

//	case 0x50:	// f_mount (included in Int 13h)
//		result = f_mount(&sdcard, "", 1);
//		break;

	case 0x51:	// f_open
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_open(&fatfsfile[fileno], rwbuf, bcnt & 0xFF);
		break;

	case 0x52:	// f_close
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_close(&fatfsfile[fileno]);
		break;

	case 0x53:	// f_read
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_read(&fatfsfile[fileno], rwbuf, bcnt, &bread);
		r.dx = bread;
		break;

	case 0x54:	// f_write
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_write(&fatfsfile[fileno], rwbuf, bcnt, &bread);
		r.dx = bread;
		break;

	case 0x55:	// f_lseek
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_lseek(&fatfsfile[fileno], offset);
		break;

//	case 0x56:	// f_truncate (excluded by _FS_MINIMIZE >= 1)
//		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
//		result = f_truncate(&fatfsfile[fileno]);
//		break;

	case 0x57:	// f_sync
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_sync(&fatfsfile[fileno]);
		break;

//	case 0x58:	// f_forward (excluded by _USE_FORWARD = 0 & _FS_TINY = 0)
//		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
//		result = f_forward(&fatfsfile[fileno], rwbuf, bcnt, &bread);
//		r.dx = bread;
//		break;

//	case 0x59:	// f_stat (excluded by _FS_MINIMIZE >= 1)
//	case 0x5A:	// f_opendir (not implemented yet, but may be useful)
//	case 0x5B:	// f_closedir (not implemented yet, but may be useful)
//	case 0x5C:	// f_readdir (not implemented yet, but may be useful)

//	case 0x5D:	// f_mkdir (excluded by _FS_MINIMIZE >= 1)
//		result = f_mkdir(rwbuf);
//		break;

//	case 0x5E:	// f_unlink (excluded by _FS_MINIMIZE >= 1)
//		result = f_unlink(rwbuf);
//		break;

//	case 0x5F:	// f_chmod (excluded by _FS_MINIMIZE >= 1)
//		result = f_chmod(rwbuf, bcnt & 0xFF, bcnt >> 8);
//		break;

//	case 0x60:	// f_utime (excluded by _FS_MINIMIZE >= 1)
//	case 0x61:	// f_rename (excluded by _FS_MINIMIZE >= 1)

//	case 0x62:	// f_chdir (excluded by _FS_RPATH = 0)
//		result = f_chdir(rwbuf);
//		break;

//	case 0x63:	// f_chdrive (excluded by _FS_RPATH = 0)
//		result = f_chdrive(rwbuf);
//		break;

//	case 0x64:	// f_getcwd (excluded by _FS_RPATH < 2)
//		result = f_getcwd(rwbuf, bcnt);
//		break;

//	case 0x65:	// f_getfree (excluded by _FS_MINIMIZE >= 1)
//	case 0x66:	// f_getlabel (excluded by _USE_LABEL = 0)

//	case 0x67:	// f_setlabel (excluded by _USE_LABEL = 0)
//		result = f_setlabel(rwbuf);
//		break;

//	case 0x68:	// f_mkfs (excluded by _USE_MKFS = 0)
//		result = f_mkfs(rwbuf, fileno, bcnt);
//		break;

//	case 0x69:	// f_fdisk (excluded by _USE_MKFS = 0 & _MULTI_PARTITION = 0)

//	case 0x6A:	// f_gets (excluded by FF_USE_STRFUNC = 0)
//		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
//		f_gets(rwbuf, bcnt, &fatfsfile[fileno]);
//		result = f_error(&fatfsfile[fileno]) ? FR_DISK_ERR : FR_OK;
//		break;

//	case 0x6B:	// f_putc (excluded by FF_USE_STRFUNC = 0)
//		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
//		f_putc(bcnt & 0xFF, &fatfsfile[fileno]);
//		result = f_error(&fatfsfile[fileno]) ? FR_DISK_ERR : FR_OK;
//		break;

//	case 0x6C:	// f_puts (excluded by FF_USE_STRFUNC = 0)
//		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
//		f_puts(rwbuf, &fatfsfile[fileno]);
//		result = f_error(&fatfsfile[fileno]) ? FR_DISK_ERR : FR_OK;
//		break;

//	case 0x6D:	// f_printf (not implemented)

	case 0x6E:	// f_tell
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		offset = f_tell(&fatfsfile[fileno]);
		r.cx = offset >> 16;
		r.dx = offset & 0xFFFF;
		result = FR_OK;
		break;

	case 0x6F:	// f_eof
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_eof(&fatfsfile[fileno]);
		break;

	case 0x70:	// f_size
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		offset = f_size(&fatfsfile[fileno]);
		r.cx = offset >> 16;
		r.dx = offset & 0xFFFF;
		result = FR_OK;
		break;

	case 0x71:	// f_error
		if (fileno >= FILES) { result = FR_INVALID_PARAMETER; break; }
		result = f_error(&fatfsfile[fileno]);
		break;

	case 0xC0:	// Get configuration
		r.es = FP_SEG(cfgtbl);
		r.bx = FP_OFF(cfgtbl);
		result = 0x00;	// Success
		break;

	default:	// Unsupported function
		result = 0x86;
	}

	// Return with status in AH and carry set on error
	r.ax &= 0xFF;
	r.ax |= result << 8;
	if (result != FR_OK) { r.flags |= F_C; } else { r.flags &= ~F_C; }
}

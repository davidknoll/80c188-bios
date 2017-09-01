/* Diskette Drive Parameter Tables (DDPT), taken from ATBIOSV3.ZIP
 * Fixed Disk Parameter Tables (FDPT)
 */
#include "bios.h"
#define MOTOR_WAIT 37
#define RATE_500 0x00
#define RATE_300 0x40
#define RATE_250 0x80
#define RATE_RSV 0xC0	// Reserved

// 5 1/4" 360KB, 40 cyl * 2 hd * 9 spt * 512 bps
struct ddpt ddpt360 = {
	0xDF,		// 1st specify byte- head unload, step rate
	0x02,		// 2nd specify byte- head load, DMA mode
	MOTOR_WAIT,	// Motor-off wait (ticks)
	0x02,		// (128 << this) bytes per sector
	9,			// Sectors per track
	0x2A,		// Gap length
	0xFF,		// Data length
	0x50,		// Gap length for format
	0xF6,		// Fill byte for format
	15,			// Head settle time (ms)
	8,			// Motor start time (1/8s)
	39,			// Max track number
	RATE_300	// Data transfer rate
};

// 5 1/4" 1.2MB, 80 cyl * 2 hd * 15 spt * 512 bps
struct ddpt ddpt1200 = {
	0xDF,		// 1st specify byte- head unload, step rate
	0x02,		// 2nd specify byte- head load, DMA mode
	MOTOR_WAIT,	// Motor-off wait (ticks)
	0x02,		// (128 << this) bytes per sector
	15,			// Sectors per track
	0x1B,		// Gap length
	0xFF,		// Data length
	0x54,		// Gap length for format
	0xF6,		// Fill byte for format
	15,			// Head settle time (ms)
	8,			// Motor start time (1/8s)
	79,			// Max track number
	RATE_500	// Data transfer rate
};

// 3 1/2" 720KB, 80 cyl * 2 hd * 9 spt * 512 bps
struct ddpt ddpt720 = {
	0xDF,		// 1st specify byte- head unload, step rate
	0x02,		// 2nd specify byte- head load, DMA mode
	MOTOR_WAIT,	// Motor-off wait (ticks)
	0x02,		// (128 << this) bytes per sector
	9,			// Sectors per track
	0x2A,		// Gap length
	0xFF,		// Data length
	0x50,		// Gap length for format
	0xF6,		// Fill byte for format
	15,			// Head settle time (ms)
	8,			// Motor start time (1/8s)
	79,			// Max track number
	RATE_250	// Data transfer rate
};

// 3 1/2" 1.44MB, 80 cyl * 2 hd * 18 spt * 512 bps
struct ddpt ddpt1440 = {
	0xAF,		// 1st specify byte- head unload, step rate
	0x02,		// 2nd specify byte- head load, DMA mode
	MOTOR_WAIT,	// Motor-off wait (ticks)
	0x02,		// (128 << this) bytes per sector
	18,			// Sectors per track
	0x1B,		// Gap length
	0xFF,		// Data length
	0x6C,		// Gap length for format
	0xF6,		// Fill byte for format
	15,			// Head settle time (ms)
	8,			// Motor start time (1/8s)
	79,			// Max track number
	RATE_500	// Data transfer rate
};

// 3 1/2" 2.88MB, 80 cyl * 2 hd * 36 spt * 512 bps
// (copied from 1.44MB version with minimal change,
// probably not right for a physical drive)
struct ddpt ddpt2880 = {
	0xAF,		// 1st specify byte- head unload, step rate
	0x02,		// 2nd specify byte- head load, DMA mode
	MOTOR_WAIT,	// Motor-off wait (ticks)
	0x02,		// (128 << this) bytes per sector
	36,			// Sectors per track
	0x1B,		// Gap length
	0xFF,		// Data length
	0x6C,		// Gap length for format
	0xF6,		// Fill byte for format
	15,			// Head settle time (ms)
	8,			// Motor start time (1/8s)
	79,			// Max track number
	RATE_RSV	// Data transfer rate
};

// 8" 250KB, 77 cyl * 1 hd * 26 spt * 128 bps
// (copied from 360KB version with minimal change,
// probably not right for a physical drive)
struct ddpt ddpt250 = {
	0xDF,		// 1st specify byte- head unload, step rate
	0x02,		// 2nd specify byte- head load, DMA mode
	MOTOR_WAIT,	// Motor-off wait (ticks)
	0x00,		// (128 << this) bytes per sector
	26,			// Sectors per track
	0x2A,		// Gap length
	0x80,		// Data length
	0x50,		// Gap length for format
	0xF6,		// Fill byte for format
	15,			// Head settle time (ms)
	8,			// Motor start time (1/8s)
	76,			// Max track number
	RATE_250	// Data transfer rate
};

// FDPT for first HDD image file
struct fdpt deffdpt = {
	0,			// Cylinders, filled in on image mount
	255,		// Heads, standard translation
	0, 0, 0,	// Obsolete fields
	0,			// Control byte, not used in this implementation
	0, 0, 0, 0,	// Obsolete fields
	63,			// Sectors, standard translation
	0			// Reserved
};

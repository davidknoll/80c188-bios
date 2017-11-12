/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2017                                                          */
/*------------------------------------------------------------------------*/


#include "ff.h"
#include "bios.h"
#include "ioports.h"
#include <conio.h>



#if FF_USE_LFN == 3	/* Dynamic memory allocation */
#error Can't use the heap for LFN buffers, there is no heap set up

/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block (null on not enough core) */
	UINT msize		/* Number of bytes to allocate */
)
{
	return malloc(msize);	/* Allocate a new memory block with POSIX API */
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
	void* mblock	/* Pointer to the memory block to free */
)
{
	free(mblock);	/* Free the memory block with POSIX API */
}

#endif



#if FF_FS_REENTRANT	/* Mutal exclusion */

int testandset(volatile int *lockptr)
{
	int result;
	pushcli();

	result = *lockptr;
	*lockptr = 1;

	popcli();
	return result;
}

unsigned long getticks(void)
{
	unsigned long result;
	pushcli();

	result = *((volatile unsigned long far *) (BDA+0x6C));	// BIOS timer tick count

	popcli();
	return result;
}

static volatile int locks[FF_VOLUMES];

/*------------------------------------------------------------------------*/
/* Create a Synchronization Object
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object for the volume, such as semaphore and mutex.
/  When a 0 is returned, the f_mount() function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* 1:Function succeeded, 0:Could not create the sync object */
	BYTE vol,			/* Corresponding volume (logical drive number) */
	FF_SYNC_t *sobj		/* Pointer to return the created sync object */
)
{
	*sobj = locks + vol;
	**sobj = 0;
	return 1;
}


/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* 1:Function succeeded, 0:Could not delete due to an error */
	FF_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
	return 1;
}


/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* 1:Got a grant to access the volume, 0:Could not get a grant */
	FF_SYNC_t sobj	/* Sync object to wait */
)
{
	unsigned long ticklimit = getticks() + FF_FS_TIMEOUT;
	while (testandset(sobj)) {
		if (getticks() > ticklimit) {
			return 0;
		}
		hlt();
	}
	return 1;
}


/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	FF_SYNC_t sobj	/* Sync object to be signaled */
)
{
	*sobj = 0;
}

#endif


/*--------------------------------------------------------------------*/
/* User Provided Timer Function for FatFs module                      */
/*--------------------------------------------------------------------*/
/* This is a real time clock service to be called from FatFs module.  */
/* Any valid time must be returned even if the system does not        */
/* support a real time clock. This is not required when FatFs is      */
/* configured for FF_FS_READONLY or FF_FS_NORTC = 1.                  */
/*--------------------------------------------------------------------*/

#if !FF_FS_NORTC && !FF_FS_READONLY
DWORD get_fattime (void)
{
	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(bcdtobin(inportb(RTC_YR)) + 20) << 25)	/* Year since 1980 */
			| ((DWORD) bcdtobin(inportb(RTC_MTH))      << 21)	/* Month */
			| ((DWORD) bcdtobin(inportb(RTC_DATE))     << 16)	/* Date */
			| ((DWORD) bcdtobin(inportb(RTC_HR))       << 11)	/* Hour */
			| ((DWORD) bcdtobin(inportb(RTC_MIN))      <<  5)	/* Min */
			| ((DWORD) bcdtobin(inportb(RTC_SEC))      >>  1);	/* Sec */
}
#endif


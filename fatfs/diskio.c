#include <hy18.h>
#include "ff.h"
#include "diskio.h"

static volatile unsigned char sdc_inited = 0;

DSTATUS disk_initialize(BYTE pdrv)
{
	if (pdrv > 0) { return STA_NOINIT; }
	sdc_inited = sdc_init() ? 0 : 1;
	return disk_status(pdrv);
}

DSTATUS disk_status(BYTE pdrv)
{
	unsigned long ocr;
	unsigned char csd[16];
	if (pdrv > 0) { return STA_NOINIT; }
	if (!sdc_inited) { return STA_NOINIT; }

	if (sdc_readocr(&ocr)) { return STA_NOINIT; }
	if (!(ocr & 0x80000000)) { return STA_NOINIT; }
	if (sdc_readcsd(csd)) { return STA_NOINIT; }
	if (csd[14] & 0x30) { return STA_PROTECT; }
	return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count)
{
	if (pdrv > 0) { return RES_PARERR; }
	if (!sdc_inited) { return RES_NOTRDY; }

	while (count--) {
		if (sdc_readsector(buff, sector)) {
			return RES_ERROR;
		}
		buff += SD_SECTOR_SIZE;
		sector++;
	}
	return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count)
{
	if (pdrv > 0) { return RES_PARERR; }
	if (!sdc_inited) { return RES_NOTRDY; }

	while (count--) {
		if (sdc_writesector((unsigned char *) buff, sector)) {
			if (disk_status(pdrv) & STA_PROTECT) {
				return RES_WRPRT;
			} else {
				return RES_ERROR;
			}
		}
		buff += SD_SECTOR_SIZE;
		sector++;
	}
	return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
	unsigned long l;
	if (pdrv > 0) { return RES_PARERR; }
	if (!sdc_inited) { return RES_NOTRDY; }

	switch (cmd) {
		case CTRL_SYNC:
			return RES_OK;

		case GET_SECTOR_COUNT:
			*((LBA_t *) buff) = sdc_getsectors();
			if (*((LBA_t *) buff)) {
				return RES_OK;
			} else {
				return RES_ERROR;
			}

		case GET_SECTOR_SIZE:
			*((WORD *) buff) = SD_SECTOR_SIZE;
			return RES_OK;

		case GET_BLOCK_SIZE:
			*((DWORD *) buff) = 1;
			return RES_OK;

		case CTRL_TRIM:
			if (sdc_discard(((LBA_t *) buff)[0], ((LBA_t *) buff)[1])) {
				return RES_ERROR;
			} else {
				return RES_OK;
			}

		case MMC_GET_TYPE:
			*((BYTE *) buff) = 0x0C;
			return RES_OK;

		case MMC_GET_CSD:
			if (sdc_readcsd(buff)) {
				return RES_ERROR;
			} else {
				return RES_OK;
			}

		case MMC_GET_CID:
			if (sdc_readcid(buff)) {
				return RES_ERROR;
			} else {
				return RES_OK;
			}

		case MMC_GET_OCR:
			if (sdc_readocr(&l)) {
				return RES_ERROR;
			} else {
				((BYTE *) buff)[0] = l >> 24;
				((BYTE *) buff)[1] = l >> 16;
				((BYTE *) buff)[2] = l >> 8;
				((BYTE *) buff)[3] = l;
				return RES_OK;
			}

		case MMC_GET_SDSTAT:
			if (sdc_readstatus(buff)) {
				return RES_ERROR;
			} else {
				return RES_OK;
			}

		default:
			return RES_PARERR;
	}
}

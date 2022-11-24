#include <conio.h>
#include "hy18.h"
#include "ioports.h"

/* Begin SPI layer */

/*
 * 8255 pin mapping:
 *  PB0 - SD MISO
 *  PC0 - SD MOSI / LCD SDA
 *  PC1 - SD SCLK / LCD SCK
 *  PC2 - SD /CS
 *  PC3 - PPI INTRA
 *  PC4 - LCD A0
 *  PC5 - LCD /CS
 *  PC6 - PPI /ACKA
 *  PC7 - PPI /OBFA
 */
#define DO     (inportb(PPI_PB)       & (1 << 0))  /* Test for MMC DO ('H':true, 'L':false) */
#define DI_H() outportb(PPI_CTL, 0x01 | (0 << 1))  /* Set MMC DI "high" */
#define DI_L() outportb(PPI_CTL, 0x00 | (0 << 1))  /* Set MMC DI "low" */
#define CK_H() outportb(PPI_CTL, 0x01 | (1 << 1))  /* Set MMC SCLK "high" */
#define	CK_L() outportb(PPI_CTL, 0x00 | (1 << 1))  /* Set MMC SCLK "low" */
#define	CS_H() outportb(PPI_CTL, 0x01 | (2 << 1))  /* Set MMC CS "high" */
#define CS_L() outportb(PPI_CTL, 0x00 | (2 << 1))  /* Set MMC CS "low" */

// Send/receive 1 byte full-duplex
unsigned char spi_iob(unsigned char obyte)
{
  int i;
  unsigned char ibyte = 0x00;

  for (i = 0; i < 8; i++) {
    if (obyte & 0x80) { DI_H(); } else { DI_L(); }
    obyte <<= 1;
    CK_H();
    ibyte <<= 1;
    if (DO) { ibyte |= 1; }
    CK_L();
  }

  return ibyte;
}

// Reset the SPI hardware
void spi_init(void)
{
  int i;
  // Port A = mode 1 output
  // Port B = mode 0 input
  // Port C = all output
  outportb(PPI_CTL, 0xA2); // Mode
  outportb(PPI_CTL, 0x0B); // Set LCD /CS
  outportb(PPI_CTL, 0x05); // Set SD /CS
  for (i = 0; i < 10; i++) { spi_iob(0xFF); }
}

/* Begin SD command layer */

// Generated using code from https://github.com/hazelnusse/crc7
const unsigned char crc7_table[] = {
  0x00, 0x09, 0x12, 0x1B, 0x24, 0x2D, 0x36, 0x3F, 0x48, 0x41, 0x5A, 0x53, 0x6C, 0x65, 0x7E, 0x77,
  0x19, 0x10, 0x0B, 0x02, 0x3D, 0x34, 0x2F, 0x26, 0x51, 0x58, 0x43, 0x4A, 0x75, 0x7C, 0x67, 0x6E,
  0x32, 0x3B, 0x20, 0x29, 0x16, 0x1F, 0x04, 0x0D, 0x7A, 0x73, 0x68, 0x61, 0x5E, 0x57, 0x4C, 0x45,
  0x2B, 0x22, 0x39, 0x30, 0x0F, 0x06, 0x1D, 0x14, 0x63, 0x6A, 0x71, 0x78, 0x47, 0x4E, 0x55, 0x5C,
  0x64, 0x6D, 0x76, 0x7F, 0x40, 0x49, 0x52, 0x5B, 0x2C, 0x25, 0x3E, 0x37, 0x08, 0x01, 0x1A, 0x13,
  0x7D, 0x74, 0x6F, 0x66, 0x59, 0x50, 0x4B, 0x42, 0x35, 0x3C, 0x27, 0x2E, 0x11, 0x18, 0x03, 0x0A,
  0x56, 0x5F, 0x44, 0x4D, 0x72, 0x7B, 0x60, 0x69, 0x1E, 0x17, 0x0C, 0x05, 0x3A, 0x33, 0x28, 0x21,
  0x4F, 0x46, 0x5D, 0x54, 0x6B, 0x62, 0x79, 0x70, 0x07, 0x0E, 0x15, 0x1C, 0x23, 0x2A, 0x31, 0x38,
  0x41, 0x48, 0x53, 0x5A, 0x65, 0x6C, 0x77, 0x7E, 0x09, 0x00, 0x1B, 0x12, 0x2D, 0x24, 0x3F, 0x36,
  0x58, 0x51, 0x4A, 0x43, 0x7C, 0x75, 0x6E, 0x67, 0x10, 0x19, 0x02, 0x0B, 0x34, 0x3D, 0x26, 0x2F,
  0x73, 0x7A, 0x61, 0x68, 0x57, 0x5E, 0x45, 0x4C, 0x3B, 0x32, 0x29, 0x20, 0x1F, 0x16, 0x0D, 0x04,
  0x6A, 0x63, 0x78, 0x71, 0x4E, 0x47, 0x5C, 0x55, 0x22, 0x2B, 0x30, 0x39, 0x06, 0x0F, 0x14, 0x1D,
  0x25, 0x2C, 0x37, 0x3E, 0x01, 0x08, 0x13, 0x1A, 0x6D, 0x64, 0x7F, 0x76, 0x49, 0x40, 0x5B, 0x52,
  0x3C, 0x35, 0x2E, 0x27, 0x18, 0x11, 0x0A, 0x03, 0x74, 0x7D, 0x66, 0x6F, 0x50, 0x59, 0x42, 0x4B,
  0x17, 0x1E, 0x05, 0x0C, 0x33, 0x3A, 0x21, 0x28, 0x5F, 0x56, 0x4D, 0x44, 0x7B, 0x72, 0x69, 0x60,
  0x0E, 0x07, 0x1C, 0x15, 0x2A, 0x23, 0x38, 0x31, 0x46, 0x4F, 0x54, 0x5D, 0x62, 0x6B, 0x70, 0x79
};

// Update a running CRC as used by the 6-byte SD command packets
unsigned char crc7_add(unsigned char crc, unsigned char data)
{
  return crc7_table[(crc << 1) ^ data];
}

// Send a command to the SD card, wait for first response byte, leave /CS low
unsigned char sd_sendcmd(unsigned char cmd, unsigned long arg)
{
  int tries = SD_MAX_TRIES;
  unsigned char crc, result;
  crc = crc7_add(  0, cmd | 0x40);
  crc = crc7_add(crc, arg >> 24);
  crc = crc7_add(crc, arg >> 16);
  crc = crc7_add(crc, arg >> 8);
  crc = crc7_add(crc, arg);

  spi_iob(0xFF);
  CS_L();
  spi_iob(0xFF);

  spi_iob(cmd | 0x40);
  spi_iob(arg >> 24);
  spi_iob(arg >> 16);
  spi_iob(arg >> 8);
  spi_iob(arg);
  spi_iob((crc << 1) | 0x01);

  // Wait for a response
  if (cmd == 12) { spi_iob(0xFF); }
  do {
    result = spi_iob(0xFF);
  } while ((result == 0xFF) && tries--);
  return result; // Still 0xFF if timed out
}

// Return /CS high
void sd_release(void)
{
  spi_iob(0xFF);
  CS_H();
  spi_iob(0xFF);
}

// Attempt to read a data block, return the token
unsigned char sd_readdata(unsigned char *buf, unsigned int len)
{
  int tries = SD_MAX_TRIES;
  unsigned char token;
  do {
    token = spi_iob(0xFF);
  } while ((token == 0xFF) && tries--);

  if (token == 0xFE) { // Start token
    while (len--) { *buf++ = spi_iob(0xFF); }
    spi_iob(0xFF); // Read and discard CRC
    spi_iob(0xFF);
  }
  return token;
}

// Attempt to write a data block, return the token
unsigned char sd_writedata(unsigned char *buf, unsigned int len, unsigned char token)
{
  int tries = SD_MAX_TRIES;
  spi_iob(0xFF);  // Dummy byte
  spi_iob(token); // Start token
  while (len--) { spi_iob(*buf++); }

  do {
    token = spi_iob(0xFF);
  } while ((token == 0xFF) && tries--);

  if ((token & 0x1F) == 0x05) { // Data accepted token
    tries = SD_MAX_TRIES;
    while (!spi_iob(0xFF) && tries--);
    if (!tries) { token = 0x00; }
  }
  return token;
}

// Read a further 32-bit response
unsigned long sd_response(void)
{
  unsigned long result;
  result = spi_iob(0xFF);
  result <<= 8;
  result |= spi_iob(0xFF);
  result <<= 8;
  result |= spi_iob(0xFF);
  result <<= 8;
  result |= spi_iob(0xFF);
  return result;
}

/* Begin SD sector layer */

// Read one sector
//    00h on success
//  < 80h on R1 error
// >= 80h on read data error
//    FFh on timeout waiting for R1 or for start token
unsigned char sdc_readsector(unsigned char *buf, unsigned long lba)
{
  unsigned char result = sd_sendcmd(17, lba);
  if (result) {
    sd_release();
    return result; // Error returned in R1 (bit 7 always 0)
  } else {
    result = sd_readdata(buf, SD_SECTOR_SIZE);
    sd_release();
    if (result == 0xFE) {
      return 0; // Success
    } else {
      return result | 0x80; // Error token reading data (bit 7 also 0, so we distinguish it)
    }
  }
}

unsigned char sdc_readsectors(unsigned char *buf, unsigned long lba, unsigned int count)
{
  int tries = SD_MAX_TRIES;
  unsigned char result = sd_sendcmd(18, lba);
  if (result) {
    sd_release();
    return result; // Error returned in R1 (bit 7 always 0)
  } else {
    while (count--) {
      result = sd_readdata(buf, SD_SECTOR_SIZE);
      buf += SD_SECTOR_SIZE;
      if (result != 0xFE) {
        sd_release();
        return result | 0x80; // Error token reading data (bit 7 also 0, so we distinguish it)
      }
    }

    sd_sendcmd(12, 0); // Stop transmission
    while (!spi_iob(0xFF) && tries--); // Busy wait
    sd_release();
    return 0;
  }
}

// Write one sector
//    00h on success
//  < 80h on R1 error
//    80h on timeout waiting for busy tokens to end
//  > 80h on write data error
//    FFh on timeout waiting for R1 or for data accepted token
unsigned char sdc_writesector(unsigned char *buf, unsigned long lba)
{
  unsigned char result = sd_sendcmd(24, lba);
  if (result) {
    sd_release();
    return result; // Error returned in R1 (bit 7 always 0)
  } else {
    result = sd_writedata(buf, SD_SECTOR_SIZE, 0xFE);
    sd_release();
    if ((result & 0x1F) == 0x05) {
      return 0; // Success
    } else {
      return result | 0x80; // Error token writing data
    }
  }
}

unsigned char sdc_writesectors(unsigned char *buf, unsigned long lba, unsigned int count)
{
  int tries = SD_MAX_TRIES;
  unsigned char result = sd_sendcmd(25, lba);
  if (result) {
    sd_release();
    return result; // Error returned in R1 (bit 7 always 0)
  } else {
    while (count--) {
      result = sd_writedata(buf, SD_SECTOR_SIZE, 0xFC);
      buf += SD_SECTOR_SIZE;
      if ((result & 0x1F) != 0x05) {
        sd_release();
        return result | 0x80; // Error token writing data
      }
    }

    spi_iob(0xFF);
    spi_iob(0xFD); // Stop transmission
    spi_iob(0xFF);
    while (!spi_iob(0xFF) && tries--); // Busy wait
    sd_release();
    return 0; // Success
  }
}

// Read the card's CSD register
unsigned char sdc_readcsd(unsigned char *buf)
{
  unsigned char result = sd_sendcmd(9, 0);
  if (result) {
    sd_release();
    return result; // Error returned in R1 (bit 7 always 0)
  } else {
    result = sd_readdata(buf, 16);
    sd_release();
    if (result == 0xFE) {
      return 0; // Success
    } else {
      return result | 0x80; // Error token reading data (bit 7 also 0, so we distinguish it)
    }
  }
}

// Read the card's CID register
unsigned char sdc_readcid(unsigned char *buf)
{
  unsigned char result = sd_sendcmd(10, 0);
  if (result) {
    sd_release();
    return result; // Error returned in R1 (bit 7 always 0)
  } else {
    result = sd_readdata(buf, 16);
    sd_release();
    if (result == 0xFE) {
      return 0; // Success
    } else {
      return result | 0x80; // Error token reading data (bit 7 also 0, so we distinguish it)
    }
  }
}

// Return the number of 512-byte sectors on the device (max LBA + 1), 0 on error
unsigned long sdc_getsectors(void)
{
  unsigned char csd[16];
  unsigned long sectors;
  if (sdc_readcsd(csd)) { return 0; } // Error
  sectors = csd[7] & 0x3F;
  sectors <<= 8;
  sectors |= csd[8];
  sectors <<= 8;
  sectors |= csd[9];
  sectors++;
  sectors <<= 10;
  return sectors;
}

// Read the card's OCR register
unsigned char sdc_readocr(unsigned long *ocr)
{
  unsigned char r1 = sd_sendcmd(58, 0);
  *ocr = sd_response();
  sd_release();
  return r1;
}

// Inform the card that a range of sectors can be erased
//    00h on success
//  < 80h if R1 error setting addresses or erase
//    80h if timed-out busy-waiting on the erase, but success was otherwise indicated in R1
//  > 80h if timed-out busy-waiting on the erase, plus some other error
//    FEh if the OCR says the card hasn't been initialised
unsigned char sdc_discard(unsigned long lbastart, unsigned long lbaend)
{
  unsigned long ocr;
  unsigned char result = sdc_readocr(&ocr);
  int tries = SD_MAX_TRIES;

  if (result) { return result; }
  if (!(ocr & 0x80000000)) { return 0xFE; } // Error- card not inited
  if (ocr & 0x40000000) { // SDHC / SDXC
    result = sd_sendcmd(32, lbastart); // Units are sectors
    if (result) { return result; }
    result = sd_sendcmd(33, lbaend);
    if (result) { return result; }
  } else { // SDSC
    result = sd_sendcmd(32, lbastart * SD_SECTOR_SIZE); // Units are bytes
    if (result) { return result; }
    result = sd_sendcmd(33, lbaend * SD_SECTOR_SIZE);
    if (result) { return result; }
  }

  result = sd_sendcmd(38, 0); // Erase
  while (!spi_iob(0xFF) && tries--);
  sd_release();
  if (!tries) { result |= 0x80; }
  return result;
}

// Read the card's status register
unsigned int sdc_readstatus(unsigned char *buf)
{
  unsigned int result = sd_sendcmd(55, 0);
  if (result) {
    sd_release();
    return result; // Error returned in R1 (bit 7 always 0)
  }

  result = sd_sendcmd(13, 0); // R1
  result <<= 8;
  result |= spi_iob(0xFF); // R2
  if (result) {
    sd_release();
    return result; // Error returned in R2
  } else {
    result = sd_readdata(buf, 64);
    sd_release();
    if (result == 0xFE) {
      return 0; // Success
    } else {
      return result | 0x80; // Error token reading data (bit 7 also 0, so we distinguish it)
    }
  }
}

// Power-up initialisation
unsigned char sdc_init(void)
{
  unsigned char r1;
  unsigned long r37;
  int tries = SD_MAX_TRIES;
  spi_init();

  r1 = sd_sendcmd(0, 0);
  if (r1 > 0x01) {
    sd_release();
    return r1; // Error- unknown card / failed on reset
  }
  r1 = sd_sendcmd(8, 0x000001AA);
  if (r1 > 0x01) {
    sd_release();
    return r1; // "Error"- SDv1 / MMCv3 not implemented yet
  }
  r37 = sd_response();
  if ((r37 & 0x00000FFF) != 0x1AA) {
    sd_release();
    return 0xFE; // Error- unknown card / check pattern mismatch
  }

  do {
    r1 = sd_sendcmd(55, 0);
    if (r1 > 0x01) {
      sd_release();
      return r1;
    }
    r1 = sd_sendcmd(41, 0x40000000);
    if (r1 > 0x01) {
      sd_release();
      return r1; // Error- failed initialising card
    }
  } while ((r1 == 0x01) && tries--);
  if (!tries) {
    sd_release();
    return 0xFF; // Error- timeout initialising card
  }

  r1 = sd_sendcmd(58, 0);
  if (r1) {
    sd_release();
    return r1;
  }
  r37 = sd_response();
  if (!(r37 & 0x80000000)) { // Power up status
    sd_release();
    return 0x80; // Error- card not finished init despite successful ACMD41
  }
  if (!(r37 & 0x40000000)) { // CCS
    // At this point we have a SDv2 SDSC- but the below will be needed for MMCv3 / SDv1 too
    r1 = sd_sendcmd(16, SD_SECTOR_SIZE); // Set sector size
  }
  sd_release();
  return r1; // 00h on success, otherwise on error
}

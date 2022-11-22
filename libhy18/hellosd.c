#include <hy18.h>
#include "iofunc.h"

void hexdump(unsigned char *buf, unsigned int len)
{
  int i, j, rows = len >> 4;
  unsigned char k;
  if (len & 0xF) { rows++; } // Round up to nearest whole row of 16 bytes

  for (i = 0; i < rows; i++) {
    serouthw(i << 4); // Address
    outstr("  ");

    for (j = 0; j < 16; j++) {
      serouthb(buf[(i * 16) + j]); // Hex bytes
      if ((j == 7) || (j == 15)) { seroutb(' '); }
      seroutb(' ');
    }

    for (j = 0; j < 16; j++) {
      k = buf[(i * 16) + j];
      if ((k < 0x20) || ((k >= 0x7F) && (k < 0xA0))) {
        seroutb('.'); // Non-printable characters
      } else {
        seroutb(k); // Printable characters
      }
    }
    outstr("\r\n");
  }
}

int main()
{
  unsigned char secbuf[SD_SECTOR_SIZE];
  outstr("\r\nSD card test\r\n");

  outstr("Init result: ");
  serouthb(sdc_init());
  outstr("\r\n");

  outstr("Sectors: ");
  serouthl(sdc_getsectors());
  outstr("\r\n");

  outstr("Read CSD, R1: ");
  serouthb(sdc_readcsd(secbuf));
  outstr(", CSD:\r\n");
  hexdump(secbuf, 16);

  outstr("Read CID, R1: ");
  serouthb(sdc_readcid(secbuf));
  outstr(", CID:\r\n");
  hexdump(secbuf, 16);

  outstr("Read status, R2: ");
  serouthw(sdc_readstatus(secbuf));
  outstr(", status:\r\n");
  hexdump(secbuf, 64);

  outstr("Read sector, R1: ");
  serouthb(sdc_readsector(secbuf, 0));
  outstr(", hexdump of MBR:\r\n");
  hexdump(secbuf, 512);

  outstr("Done\r\n");
  return 0;
}

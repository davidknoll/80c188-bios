/**
 * HY-1.8 SPI LCD driver for David's 6309-6829 SBC
 * SPI communication layer
 */
#include <conio.h>
#include "hy18.h"
#include "ioports.h"

static void lcdspioutb(unsigned char c)
{
  int i;
  for (i = 0; i < 8; i++) {
    outportb(PPI_CTL, 0x02); // Clear SCK
    if (c & 0x80) {
      outportb(PPI_CTL, 0x01); // Set SDA
    } else {
      outportb(PPI_CTL, 0x00); // Clear SDA
    }
    outportb(PPI_CTL, 0x03); // Set SCK
    c <<= 1;
  }
}

void lcdoutbn(const unsigned char *buf, unsigned int len)
{
  while (len--) {
    lcdspioutb(*buf++);
  }
}

void lcdoutwr(unsigned int w, unsigned int cnt)
{
  while (cnt--) {
    lcdspioutb(w >> 8);
    lcdspioutb(w);
  }
}

void lcdreset(void)
{
  int i;
  outportb(PPI_CTL, (inportb(PPI_CTL) & ~0x09) | 0x80); // Port C output
  outportb(PPI_CTL, 0x0B); // Set LCD /CS
  outportb(PPI_CTL, 0x05); // Set SD /CS
  outportb(PPI_CTL, 0x03); // Set SCK
  for (i = 0; i < 500; i++);
}

void lcdsetcmd(void)   { outportb(PPI_CTL, 0x08); } // Clear A0
void lcdsetdat(void)   { outportb(PPI_CTL, 0x09); } // Set A0
void lcdspibegin(void) { outportb(PPI_CTL, 0x0A); } // Clear LCD /CS
void lcdspiend(void)   { outportb(PPI_CTL, 0x0B); } // Clear LCD /CS

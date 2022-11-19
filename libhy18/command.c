/**
 * HY-1.8 SPI LCD driver for David's 6309-6829 SBC
 * ST7735 command layer
 * Init sequence from https://github.com/stefangordon/Netduino-HY-1.8-SPI-TFT-Driver
 */
#include <stdlib.h>
#include "hy18.h"

void lcdcmd(unsigned char cmd, const unsigned char *data, int datalen, int delay)
{
  int i;
  lcdspibegin();
  lcdsetcmd();
  lcdoutbn(&cmd, 1);
  lcdsetdat();
  lcdoutbn(data, datalen);
  lcdspiend();
  for (i = 0; i < delay; i++);
}

void lcdsetaddresswindow(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1)
{
  unsigned char d_caset[4] = { 0x00, /*x0*/ 0x00, 0x00, /*x1*/ 0x00 };
  unsigned char d_raset[4] = { 0x00, /*y0*/ 0x00, 0x00, /*y1*/ 0x00 };
  d_caset[1] = x0;
  d_caset[3] = x1;
  d_raset[1] = y0;
  d_raset[3] = y1;

  lcdcmd(C_CASET, d_caset, 4, 0);
  lcdcmd(C_RASET, d_raset, 4, 0);
  lcdcmd(C_RAMWR, NULL, 0, 0);
}

void lcdinit(void)
{
  const unsigned char d_frmctr1[3] = { 0x01, 0x2C, 0x2D };
  const unsigned char d_frmctr2[3] = { 0x01, 0x2C, 0x2D };
  const unsigned char d_frmctr3[6] = { 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D };
  const unsigned char d_invctr[1]  = { 0x07 };
  const unsigned char d_pwctr1[3]  = { 0xA2, 0x02, 0x84 };
  const unsigned char d_pwctr2[1]  = { 0xC5 };
  const unsigned char d_pwctr3[2]  = { 0x0A, 0x00 };
  const unsigned char d_pwctr4[2]  = { 0x8A, 0x2A };
  const unsigned char d_pwctr5[2]  = { 0x8A, 0xEE };
  const unsigned char d_vmctr1[1]  = { 0x0E };
  const unsigned char d_madctl[1]  = { 0xC8 };
  const unsigned char d_colmod[1]  = { 0x05 };
  const unsigned char d_caset[4]   = { 0x00, 0x00, 0x00, HEIGHT - 1 };
  const unsigned char d_raset[4]   = { 0x00, 0x00, 0x00, WIDTH - 1 };
  const unsigned char d_gmctrp1[16] = {
    0x02, 0x1c, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2d,
    0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10
  };
  const unsigned char d_gmctrn1[16] = {
    0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
    0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10
  };
  const unsigned char d_vsclldef[6] = { 0x00, 0x00, 0x00, WIDTH, 0x00, 0x00 };

  // Note HEIGHT and WIDTH reversed as we present it as landscape higher up
  lcdreset();
  lcdcmd(C_SWRESET, NULL, 0, 1500);
  lcdcmd(C_SLPOUT,  NULL, 0, 1500);
  lcdcmd(C_FRMCTR1, d_frmctr1, 3, 0);
  lcdcmd(C_FRMCTR2, d_frmctr2, 3, 0);
  lcdcmd(C_FRMCTR3, d_frmctr3, 6, 0);
  lcdcmd(C_INVCTR,  d_invctr, 1, 0);
  lcdcmd(C_PWCTR1,  d_pwctr1, 3, 0);
  lcdcmd(C_PWCTR2,  d_pwctr2, 1, 0);
  lcdcmd(C_PWCTR3,  d_pwctr3, 2, 0);
  lcdcmd(C_PWCTR4,  d_pwctr4, 2, 0);
  lcdcmd(C_PWCTR5,  d_pwctr5, 2, 0);
  lcdcmd(C_VMCTR1,  d_vmctr1, 1, 0);
  lcdcmd(C_INVOFF,  NULL, 0, 0);
  lcdcmd(C_MADCTL,  d_madctl, 1, 0);
  lcdcmd(C_COLMOD,  d_colmod, 1, 0);
  lcdcmd(C_CASET,   d_caset, 4, 0);
  lcdcmd(C_RASET,   d_raset, 4, 0);
  lcdcmd(C_GMCTRP1, d_gmctrp1, 16, 0);
  lcdcmd(C_GMCTRN1, d_gmctrn1, 16, 0);
  lcdcmd(C_VSCLLDEF, d_vsclldef, 6, 0);
  lcdcmd(C_DISPON, NULL, 0, 500);
  lcdcmd(C_NORON,  NULL, 0, 100);

  lcdsetaddresswindow(0, 0, HEIGHT - 1, WIDTH - 1);
}

void lcdsethscroll(int hs)
{
  unsigned char d_vsstadrs[2] = { /*hs >> 8*/ 0x00, /*hs*/ 0x00 };
  d_vsstadrs[0] = hs >> 8;
  d_vsstadrs[1] = hs;

  hs = (WIDTH - hs) % WIDTH;
  lcdcmd(C_VSSTADRS, d_vsstadrs, 2, 0);
}

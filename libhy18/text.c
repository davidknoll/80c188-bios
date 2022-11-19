/**
 * HY-1.8 SPI LCD driver for David's 6309-6829 SBC
 * Text rendering functions (rotated for landscape viewing)
 */
#include "hy18.h"

void gfx_textchartransparent(char c, int x, int y, int fgcol)
{
  int tx, ty;
  unsigned char *ptr = font_bin + (c * 6);

  for (tx = x; tx < x + 6; tx++) {
    unsigned char b = *ptr++;
    for (ty = y; ty < y + 8; ty++) {
      if (b & 1) { gfx_plot(tx, ty, fgcol); }
      b >>= 1;
    }
  }
}

void gfx_textcharopaque(char c, int x, int y, int fgcol, int bgcol)
{
  // Framebuffer scan direction
  // Top-Bottom > Left-Right physical / portrait =
  // Right-Left > Top-Bottom logical / landscape
  int i, j, fgbgr = RGBTOBGR565(fgcol), bgbgr = RGBTOBGR565(bgcol);
  unsigned char *ptr = font_bin + (c * 6) + 5;
  if (x < 0 || x + 5 >= WIDTH || y < 0 || y + 7 >= HEIGHT) { return; }
  lcdsetaddresswindow(y, (WIDTH - 1) - (x + 5), y + 7, (WIDTH - 1) - x);

  lcdspibegin();
  for (i = 0; i < 6; i++) {
    unsigned char b = *ptr--;
    for (j = 0; j < 8; j++) {
      lcdoutwr((b & 1) ? fgbgr : bgbgr, 1);
      b >>= 1;
    }
  }
  lcdspiend();
}

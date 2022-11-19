/**
 * HY-1.8 SPI LCD driver for David's 6309-6829 SBC
 * Terminal emulation (portrait, due to hardware scroll)
 */
#include <stdlib.h>
#include "hy18.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define NUMESCPAR 8

static char txtrow = 0, txtrowsave = 0;
static char txtcol = 0, txtcolsave = 0;
static char txtscroll = 0;
static int  txtfgbgr = RGBTOBGR565(WHITE);
static int  txtbgbgr = RGBTOBGR565(BLACK);
static int  txtcubgr = RGBTOBGR565(RED);
static char escstate = 0;
static int  escparcnt;
static int  escparams[NUMESCPAR];
static int  sgrattrs = 0;

// Draw (or effectively undraw, depending on colour) the cursor in position
static void drawcursor(int bgr)
{
  lcdsetaddresswindow(
    txtcol * 6, ((txtrow * 8) + txtscroll) % WIDTH,
    txtcol * 6, ((txtrow * 8) + txtscroll + 7) % WIDTH
  );
  lcdspibegin();
  lcdoutwr(bgr, 8);
  lcdspiend();
}

// Blank an area defined by text rows / columns zero-based inclusive
static void blankarea(char r0, char c0, char r1, char c1)
{
  char r0px = ((r0 * 8) + txtscroll) % WIDTH;
  char r1px = ((r1 * 8) + txtscroll + 7) % WIDTH;
  char cwpx = ((c1 - c0) + 1) * 6;

  // Does the area being cleared cross the scroll boundary?
  if (r0px > r1px) {
    lcdsetaddresswindow(c0 * 6, r0px, (c1 * 6) + 5, WIDTH - 1);
    lcdspibegin();
    lcdoutwr(txtbgbgr, (WIDTH - r0px) * cwpx);
    lcdspiend();

    lcdsetaddresswindow(c0 * 6, 0, (c1 * 6) + 5, r1px);
    lcdspibegin();
    lcdoutwr(txtbgbgr, (r1px + 1) * cwpx);
    lcdspiend();

  } else {
    lcdsetaddresswindow(c0 * 6, r0px, (c1 * 6) + 5, r1px);
    lcdspibegin();
    lcdoutwr(txtbgbgr, ((r1px - r0px) + 1) * cwpx);
    lcdspiend();
  }
}

// Scroll a line when you're at the bottom of the screen
static void scrollbottom(void)
{
  blankarea(0, 0, 0, TXTW - 1);
  txtscroll += 8;
  txtscroll %= WIDTH;
  lcdsethscroll(txtscroll);
}

// Scroll a line when you're at the top of the screen
static void scrolltop(void)
{
  txtscroll -= 8;
  txtscroll %= WIDTH;
  lcdsethscroll(txtscroll);
  blankarea(0, 0, 0, TXTW - 1);
}

// Process C0/C1 control characters
static void ctrlchar(char c)
{
  switch (c) {
  case 0x07: // BEL
    // Flash the screen, as we have no built-in sound
    lcdcmd(C_INVON, NULL, 0, 5000);
    lcdcmd(C_INVOFF, NULL, 0, 0);
    break;

  case 0x08: // BS
    if (txtcol > 0) {
      drawcursor(txtbgbgr);
      txtcol--;
      drawcursor(txtcubgr);
    }
    break;

  case 0x09: // HT
    // Tab width must be a power of 2 due to the calculation below
    drawcursor(txtbgbgr);
    txtcol = (txtcol & ~(TXTTABW - 1)) + TXTTABW;
    if (txtcol >= TXTW) { txtcol = TXTW - 1; }
    drawcursor(txtcubgr);
    break;

  case 0x0A: // LF
  case 0x0B: // VT
  case 0x0C: // FF
  case 0x84: // IND
    drawcursor(txtbgbgr);
    txtrow++;
    if (txtrow >= TXTH) {
      txtrow = TXTH - 1;
      scrollbottom();
    }
    drawcursor(txtcubgr);
    break;

  case 0x0D: // CR
    drawcursor(txtbgbgr);
    txtcol = 0;
    drawcursor(txtcubgr);
    break;

  case 0x18: // CAN
  case 0x1A: // SUB
    escstate = 0;
    break;
  case 0x1B: // ESC
    escstate = 0x1B;
    break;

  case 0x85: // NEL
    drawcursor(txtbgbgr);
    txtcol = 0;
    txtrow++;
    if (txtrow >= TXTH) {
      txtrow = TXTH - 1;
      scrollbottom();
    }
    drawcursor(txtcubgr);
    break;

  case 0x8D: // RI
    drawcursor(txtbgbgr);
    txtrow--;
    if (txtrow < 0) {
      txtrow = 0;
      scrolltop();
    }
    drawcursor(txtcubgr);
    break;

  case 0x94: // CCH
    if (txtcol > 0) {
      drawcursor(txtbgbgr);
      txtcol--;
    }
    blankarea(txtrow, txtcol, txtrow, txtcol);
    drawcursor(txtcubgr);
    break;

  case 0x9B: // CSI
    escstate = '[';
    escparcnt = 0;
    escparams[0] = 0;
    break;

  case 0x9C: // ST
    escstate = 0;
    break;
  case 0x90: // DCS
  case 0x9D: // OSC
  case 0x9E: // PM
  case 0x9F: // APC
    escstate = 0x90;
    break;
  }
}

static void writechar(char c)
{
  int i, j, k;
  char l;

  // Write one printable character
  lcdsetaddresswindow(
    txtcol * 6, ((txtrow * 8) + txtscroll) % WIDTH,
    (txtcol * 6) + 5, ((txtrow * 8) + txtscroll + 7) % WIDTH
  );
  lcdspibegin();
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 6; j++) {
      k = (c * 6) + j;
      l = font_bin[k];
      if ((sgrattrs & (1 << 3)) && i > 3) { l  = (j < 5) ? font_bin[k + 1] : 0; } // Italic
      if ((sgrattrs & (1 << 1)) && j < 5) { l |=           font_bin[k + 1];     } // Bold
      if ((sgrattrs & (1 << 8))         ) { l  = 0;    } // Hide
      if ((sgrattrs & (1 << 4))         ) { l |= 0x80; } // Underline
      if ((sgrattrs & (1 << 9))         ) { l |= 0x10; } // Strikethrough
      if ((sgrattrs & (1 << 7))         ) { l ^= 0xFF; } // Reverse
      lcdoutwr(((l >> i) & 1) ? txtfgbgr : txtbgbgr, 1);
    }
  }
  lcdspiend();

  // Advance the cursor
  txtcol++;
  if (txtcol >= TXTW) {
    txtcol = 0;
    txtrow++;
  }
  if (txtrow >= TXTH) {
    txtrow = TXTH - 1;
    scrollbottom();
  }
  drawcursor(txtcubgr);
}

// Select Graphics Rendition
static void sgr(int attr)
{
  static const unsigned int sgrcol[] = {
    // 0b0000000000000000, 0b0000000000010101, 0b0000010101000000, 0b0000001010110101,
    // 0b1010100000000000, 0b1010100000010101, 0b1010110101000000, 0b1010110101010101,
    // 0b0101001010101010, 0b0101001010111111, 0b0101011111101010, 0b0101011111111111,
    // 0b1111101010101010, 0b1111101010111111, 0b1111111111101010, 0b1111111111111111
    0x0000,             0x0015,             0x0540,             0x02B5,
    0xA800,             0xA815,             0xAD40,             0xAD55,
    0x52AA,             0x52BF,             0x57EA,             0x57FF,
    0xFAAA,             0xFABF,             0xFFEA,             0xFFFF
  };
  if (attr >=  30 && attr <=  37) { txtfgbgr = sgrcol[attr - 30]; }
  if (attr >=  40 && attr <=  47) { txtbgbgr = sgrcol[attr - 40]; }
  if (attr >=  90 && attr <=  97) { txtfgbgr = sgrcol[attr - 82]; }
  if (attr >= 100 && attr <= 107) { txtbgbgr = sgrcol[attr - 92]; }

  switch (attr) {
  case 0: // Reset
    txtfgbgr = RGBTOBGR565(WHITE);
    txtbgbgr = RGBTOBGR565(BLACK);
    sgrattrs = 0;
    break;

  case 1: // Bold
  case 3: // Italic
  case 4: // Underline
  case 7: // Reverse
  case 8: // Hide
  case 9: // Strikethrough
    sgrattrs |= 1 << attr;
    break;
  case 21: // Normal intensity (some terminals, should be double underline)
  case 22: // Normal intensity (standard)
    sgrattrs &= ~(1 << 1);
    break;
  case 23: // Not italic
  case 24: // Not underlined
  case 27: // Not reversed
  case 28: // Not hidden
  case 29: // Not strikethrough
    sgrattrs &= ~(1 << (attr - 20));
    break;

  case 39: // Default foreground colour
    txtfgbgr = RGBTOBGR565(WHITE);
    break;
  case 49: // Default background colour
    txtbgbgr = RGBTOBGR565(BLACK);
    break;
  }
}

// Process escape sequences
static void escchar(char c)
{
  int i;
  switch (escstate) {
  case 0x1B: // 7-bit C1 controls
    switch (c) {
    case 'D': // IND
      ctrlchar(0x84);
      escstate = 0;
      break;

    case 'E': // NEL
      ctrlchar(0x85);
      escstate = 0;
      break;

    case 'M': // RI
      ctrlchar(0x8D);
      escstate = 0;
      break;

    case 'T': // CCH
      ctrlchar(0x94);
      escstate = 0;
      break;

    case '[': // CSI
      escstate = '[';
      escparcnt = 0;
      escparams[0] = 0;
      break;

    case 'P': // DCS
    case ']': // OSC
    case '^': // PM
    case '_': // APC
      escstate = 0x90;
      break;

    default: // Unrecognised / unimplemented / invalid (and ST)
      escstate = 0;
    }
    break;

  case '[': // Control sequence
    // Numeric parameter
    if (c >= '0' && c <= '9') {
      escparams[escparcnt] = (escparams[escparcnt] * 10) + (c - '0');
      break;
    }

    switch (c) {
    case ';': // Parameter separator
      if (escparcnt++ >= NUMESCPAR) {
        escstate = 0;
      } else {
        escparams[escparcnt] = 0;
      }
      break;

    case 'A': // Cursor up - ESC [ Pn A
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      txtrow -= escparams[0];
      if (txtrow < 0) { txtrow = 0; }
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'B': // Cursor down - ESC [ Pn B
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      txtrow += escparams[0];
      if (txtrow >= TXTH) { txtrow = TXTH - 1; }
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'C': // Cursor right - ESC [ Pn C
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      txtcol += escparams[0];
      if (txtcol >= TXTW) { txtcol = TXTW - 1; }
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'D': // Cursor left - ESC [ Pn D
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      txtcol -= escparams[0];
      if (txtcol < 0) { txtcol = 0; }
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'E': // Cursor next line
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      txtcol = 0;
      txtrow += escparams[0];
      if (txtrow >= TXTH) { txtrow = TXTH - 1; }
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'F': // Cursor previous line
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      txtcol = 0;
      txtrow -= escparams[0];
      if (txtrow < 0) { txtrow = 0; }
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'G': // Cursor horizontal position
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      txtcol = MAX(1, MIN(TXTW, escparams[0])) - 1;
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'H': // Cursor position - ESC [ Pn ; Pn H
    case 'f':
      if (escparcnt < 1) { escparams[1] = 0; }
      drawcursor(txtbgbgr);
      txtrow = MAX(1, MIN(TXTH, escparams[0])) - 1;
      txtcol = MAX(1, MIN(TXTW, escparams[1])) - 1;
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'J': // Erase in display
      switch (escparams[0]) {
      case 0:
        blankarea(txtrow, txtcol, txtrow, TXTW - 1);
        blankarea(txtrow + 1, 0, TXTH - 1, TXTW - 1);
        drawcursor(txtcubgr);
        break;
      case 1:
        blankarea(0, 0, txtrow - 1, TXTW - 1);
        blankarea(txtrow, 0, txtrow, txtcol);
        drawcursor(txtcubgr);
        break;
      case 2:
        blankarea(0, 0, TXTH - 1, TXTW - 1);
        drawcursor(txtcubgr);
        break;
      }
      escstate = 0;
      break;

    case 'K': // Erase in line
      switch (escparams[0]) {
      case 0:
        blankarea(txtrow, txtcol, txtrow, TXTW - 1);
        drawcursor(txtcubgr);
        break;
      case 1:
        blankarea(txtrow, 0, txtrow, txtcol);
        drawcursor(txtcubgr);
        break;
      case 2:
        blankarea(txtrow, 0, txtrow, TXTW - 1);
        drawcursor(txtcubgr);
        break;
      }
      escstate = 0;
      break;

    case 'S': // Scroll up
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      for (i = 0; i < escparams[0]; i++) { scrollbottom(); }
      drawcursor(txtcubgr);
      escstate = 0;
      break;
    case 'T': // Scroll down
      if (escparams[0] == 0) { escparams[0] = 1; }
      drawcursor(txtbgbgr);
      for (i = 0; i < escparams[0]; i++) { scrolltop(); }
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    case 'm': // SGR
      for (i = 0; i <= escparcnt; i++) { sgr(escparams[i]); }
      escstate = 0;
      break;

    case 's': // Save cursor position
      txtcolsave = txtcol;
      txtrowsave = txtrow;
      escstate = 0;
      break;
    case 'u': // Restore cursor position
      drawcursor(txtbgbgr);
      txtcol = txtcolsave;
      txtrow = txtrowsave;
      drawcursor(txtcubgr);
      escstate = 0;
      break;

    default: // Unrecognised / unimplemented / invalid
      if (c >= 0x40) { escstate = 0; }
    }
    break;

  case 0x90: // DCS/OSC/PM/APC string
    // Just goes until terminated with ST, and gets ignored
    break;

  default:
    // We're probably in an invalid state if we get here...
    escstate = 0;
  }
}

void gfx_termoutc(unsigned char c)
{
  if (c < 0x20 || (c >= 0x7F && c <= 0x9F)) {
    ctrlchar(c);
  } else if (escstate > 0) {
    escchar(c);
  } else {
    writechar(c);
  }
}

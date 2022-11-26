/**
 * Testcard demo- load a 16-bit 160x128 BMP named testcard.bmp
 * from the SD card using FatFs, to the LCD using libhy18
 */
#include <ff.h>
#include <hy18.h>
#include <iofunc.h>
// #define SWABW(w) (((w & 0xFF00) >> 8) | ((w & 0x00FF) << 8))
// #define SWABL(l) (((l & 0xFF000000L) >> 24) | ((l & 0x00FF0000L) >> 8) | ((l & 0x0000FF00L) << 8) | ((l & 0x000000FFL) << 24))
#define SWABW(w) (w)
#define SWABL(l) (l)

int main()
{
  FATFS FatFs;
  FIL Fil;
  UINT br;
  unsigned char linebuf[WIDTH * 2];
  long offset;
  int x, y;

  outstr("Testcard demo\r\n");
  lcdinit();
  gfx_clear(BLACK);
  if (f_mount(&FatFs, "", 1) != FR_OK) {
    outstr("Mount fail\r\n");
    goto end;
  }
  if (f_open(&Fil, "testcard.bmp", FA_READ | FA_OPEN_EXISTING) != FR_OK) {
    outstr("Open fail\r\n");
    goto end;
  }

  // Process the header
  f_read(&Fil, linebuf, 14 + 40, &br);
  if (br != 14 + 40) {
    outstr("Failed to read header\r\n");
    goto end;
  }
  if (
    (SWABW(*((unsigned int *)   linebuf        )) != 0x4D42) || // Windows BMP magic
    (SWABL(*((long *)          (linebuf + 0x12))) != 160)    || // Width (pixels)
    (SWABL(*((long *)          (linebuf + 0x16))) != 128)    || // Height (pixels)
    (SWABW(*((unsigned int *)  (linebuf + 0x1A))) != 1)      || // Planes
    (SWABW(*((unsigned int *)  (linebuf + 0x1C))) != 16)     || // Bits per pixel
    (SWABL(*((unsigned long *) (linebuf + 0x22))) != 0xA000)    // Bitmap data size
  ) {
    outstr("Incorrect size/parameters or not a BMP file\r\n");
    goto end;
  }
  offset = SWABL(*((long *) (linebuf + 0x0A))); // File offset to bitmap data
  f_lseek(&Fil, offset);

  // Process the image data
  // BMP files start at the bottom
  for (y = HEIGHT - 1; y >= 0; y--) {
    f_read(&Fil, linebuf, WIDTH * 2, &br);
    if (br != WIDTH * 2) {
      outstr("Failed to read image data\r\n");
      goto end;
    }

    // The LCD's framebuffer is in a different orientation
    lcdsetaddresswindow(y, 0, y, WIDTH - 1);
    lcdspibegin();
    for (x = WIDTH - 1; x >= 0; x--) {
      lcdoutwr(RGBTOBGR565(SWABW(((unsigned int *) linebuf)[x])), 1);
    }
    lcdspiend();
  }

  // Test hardware scrolling
  for (x = 0; x <= WIDTH; x++) {
    for (y = 0; y < 10000; y++);
    lcdsethscroll(x);
  }

end:
  f_close(&Fil);
  f_unmount("");
  return 0;
}

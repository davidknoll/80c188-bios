#include <hy18.h>
#include <stdlib.h>
#define RGBTO565(r,g,b) (((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F))

int main()
{
  int i;
  const char *hello = "Hello, World!";
  lcdinit();
  gfx_clear(BLACK);

  // Draw lines in all directions
  gfx_line(50, 50,  50,   0, WHITE);
  gfx_line(50, 50,  75,   0, WHITE);
  gfx_line(50, 50, 100,   0, WHITE);
  gfx_line(50, 50, 100,  25, WHITE);
  gfx_line(50, 50, 100,  50, WHITE);
  gfx_line(50, 50, 100,  75, WHITE);
  gfx_line(50, 50, 100, 100, WHITE);
  gfx_line(50, 50,  75, 100, WHITE);
  gfx_line(50, 50,  50, 100, WHITE);
  gfx_line(50, 50,  25, 100, WHITE);
  gfx_line(50, 50,   0, 100, WHITE);
  gfx_line(50, 50,   0,  75, WHITE);
  gfx_line(50, 50,   0,  50, WHITE);
  gfx_line(50, 50,   0,  25, WHITE);
  gfx_line(50, 50,   0,   0, WHITE);
  gfx_line(50, 50,  25,   0, WHITE);

  // Circles
  gfx_circlefilled(125,  20, 15, RED);
  gfx_circlefilled(125,  60, 15, GREEN);
  gfx_circlefilled(125, 100, 15, BLUE);
  gfx_circle(125,  20, 10, CYAN);
  gfx_circle(125,  60, 10, MAGENTA);
  gfx_circle(125, 100, 10, YELLOW);

  // Boxes
  gfx_boxfilled(140,  5, 159,  35, CYAN);
  gfx_boxfilled(140, 45, 159,  75, MAGENTA);
  gfx_boxfilled(140, 85, 159, 115, YELLOW);
  gfx_box(140, 10, 159,  30, RED);
  gfx_box(140, 50, 159,  70, GREEN);
  gfx_box(140, 90, 159, 110, BLUE);

  // Colour gradient
  for (i =   0; i <  25; i++) { gfx_line(i, 100, i, 120, RGBTO565(250,                  i * 10,               0)); }
  for (i =  25; i <  50; i++) { gfx_line(i, 100, i, 120, RGBTO565((25 - (i - 25)) * 10, 250,                  0)); }
  for (i =  50; i <  75; i++) { gfx_line(i, 100, i, 120, RGBTO565(0,                    250,                  (i - 50) * 10)); }
  for (i =  75; i < 100; i++) { gfx_line(i, 100, i, 120, RGBTO565(0,                    (25 - (i - 75)) * 10, 250)); }
  for (i = 100; i < 125; i++) { gfx_line(i, 100, i, 120, RGBTO565((i - 100) * 10,       0,                    250)); }
  for (i = 125; i < 150; i++) { gfx_line(i, 100, i, 120, RGBTO565(250,                  0,                    (25 - (i - 125)) * 10)); }

  // Text
  i = 0;
  while (*hello) {
    gfx_textcharopaque(*hello, i * 6, 0, GREEN, BLUE);
    gfx_textchartransparent(*hello, i * 6, 8, RED);
    hello++;
    i++;
  }

  return 0;
}

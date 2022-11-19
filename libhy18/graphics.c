/**
 * HY-1.8 SPI LCD driver for David's 6309-6829 SBC
 * Graphics functions (rotated for landscape viewing)
 * Bresenham's line algorithm ref: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
 * Bresenham's circle algorithm ref: https://www.geeksforgeeks.org/bresenhams-circle-drawing-algorithm/
 */
#include "hy18.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(a) (((a)<0)?(-(a)):(a))

void gfx_plot(int x, int y, int colour)
{
  if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) { return; }
  lcdsetaddresswindow(y, (WIDTH - 1) - x, y, (WIDTH - 1) - x);
  lcdspibegin();
  lcdoutwr(RGBTOBGR565(colour), 1);
  lcdspiend();
}

static void plotLineLow(int x0, int y0, int x1, int y1, int colour)
{
  int dx, dy, yi, D, y, x;

  dx = x1 - x0;
  dy = y1 - y0;
  yi = 1;
  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }
  D = (2 * dy) - dx;
  y = y0;

  for (x = x0; x <= x1; x++) {
    gfx_plot(x, y, colour);
    if (D > 0) {
      y += yi;
      D += 2 * (dy - dx);
    } else {
      D += 2 * dy;
    }
  }
}

static void plotLineHigh(int x0, int y0, int x1, int y1, int colour)
{
  int dx, dy, xi, D, x, y;

  dx = x1 - x0;
  dy = y1 - y0;
  xi = 1;
  if (dx < 0) {
    xi = -1;
    dx = -dx;
  }
  D = (2 * dx) - dy;
  x = x0;

  for (y = y0; y <= y1; y++) {
    gfx_plot(x, y, colour);
    if (D > 0) {
      x += xi;
      D += 2 * (dx - dy);
    } else {
      D += 2 * dx;
    }
  }
}

void gfx_line(int x0, int y0, int x1, int y1, int colour)
{
  if (x0 < 0 || x0 >= WIDTH || y0 < 0 || y0 >= HEIGHT) { return; }
  if (x1 < 0 || x1 >= WIDTH || y1 < 0 || y1 >= HEIGHT) { return; }

  if (y0 == y1) { // Horizontal line
    lcdsetaddresswindow(y0, (WIDTH - 1) - MAX(x0, x1), y0, (WIDTH - 1) - MIN(x0, x1));
    lcdspibegin();
    lcdoutwr(RGBTOBGR565(colour), ABS(x1 - x0) + 1);
    lcdspiend();

  } else if (x0 == x1) { // Vertical line
    lcdsetaddresswindow(MIN(y0, y1), (WIDTH - 1) - x0, MAX(y0, y1), (WIDTH - 1) - x0);
    lcdspibegin();
    lcdoutwr(RGBTOBGR565(colour), ABS(y1 - y0) + 1);
    lcdspiend();

  } else { // Bresenham's algorithm
    if (ABS(y1 - y0) < ABS(x1 - x0)) {
      if (x0 > x1) {
        plotLineLow(x1, y1, x0, y0, colour);
      } else {
        plotLineLow(x0, y0, x1, y1, colour);
      }
    } else {
      if (y0 > y1) {
        plotLineHigh(x1, y1, x0, y0, colour);
      } else {
        plotLineHigh(x0, y0, x1, y1, colour);
      }
    }
  }
}

void gfx_box(int x0, int y0, int x1, int y1, int colour)
{
  gfx_line(x0, y0, x1, y0, colour);
  gfx_line(x0, y1, x1, y1, colour);
  gfx_line(x0, y0, x0, y1, colour);
  gfx_line(x1, y0, x1, y1, colour);
}

void gfx_boxfilled(int x0, int y0, int x1, int y1, int colour)
{
  if (x0 < 0 || x0 >= WIDTH || y0 < 0 || y0 >= HEIGHT) { return; }
  if (x1 < 0 || x1 >= WIDTH || y1 < 0 || y1 >= HEIGHT) { return; }
  lcdsetaddresswindow(MIN(y0, y1), (WIDTH - 1) - MAX(x0, x1), MAX(y0, y1), (WIDTH - 1) - MIN(x0, x1));
  lcdspibegin();
  lcdoutwr(RGBTOBGR565(colour), (ABS(x1 - x0) + 1) * (ABS(y1 - y0) + 1));
  lcdspiend();
}

void gfx_clear(int colour)
{
  gfx_boxfilled(0, 0, WIDTH - 1, HEIGHT - 1, colour);
}

static void drawCircle(int cx, int cy, int x, int y, int colour)
{
  gfx_plot(cx+x, cy+y, colour);
  gfx_plot(cx-x, cy+y, colour);
  gfx_plot(cx+x, cy-y, colour);
  gfx_plot(cx-x, cy-y, colour);
  gfx_plot(cx+y, cy+x, colour);
  gfx_plot(cx-y, cy+x, colour);
  gfx_plot(cx+y, cy-x, colour);
  gfx_plot(cx-y, cy-x, colour);
}

static void drawCircleFilled(int cx, int cy, int x, int y, int colour)
{
  gfx_line(cx+x, cy+y, cx+x, cy, colour);
  gfx_line(cx-x, cy+y, cx-x, cy, colour);
  gfx_line(cx+x, cy-y, cx+x, cy, colour);
  gfx_line(cx-x, cy-y, cx-x, cy, colour);
  gfx_line(cx+y, cy+x, cx+y, cy, colour);
  gfx_line(cx-y, cy+x, cx-y, cy, colour);
  gfx_line(cx+y, cy-x, cx+y, cy, colour);
  gfx_line(cx-y, cy-x, cx-y, cy, colour);
}

void gfx_circle(int cx, int cy, int r, int colour)
{
  int x = 0, y = r;
  int d = 3 - 2 * r;
  drawCircle(cx, cy, x, y, colour);
  while (y >= x) {
    x++;
    if (d > 0) {
      y--;
      d = d + 4 * (x - y) + 10;
    } else {
      d = d + 4 * x + 6;
    }
    drawCircle(cx, cy, x, y, colour);
  }
}

void gfx_circlefilled(int cx, int cy, int r, int colour)
{
  int x = 0, y = r;
  int d = 3 - 2 * r;
  drawCircleFilled(cx, cy, x, y, colour);
  while (y >= x) {
    x++;
    if (d > 0) {
      y--;
      d = d + 4 * (x - y) + 10;
    } else {
      d = d + 4 * x + 6;
    }
    drawCircleFilled(cx, cy, x, y, colour);
  }
}

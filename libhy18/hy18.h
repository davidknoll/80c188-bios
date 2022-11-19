// ST7735 command codes
#define C_NOP      0x00
#define C_SWRESET  0x01
#define C_RDDID    0x04
#define C_RDDST    0x09
#define C_SLPIN    0x10
#define C_SLPOUT   0x11
#define C_PTLON    0x12
#define C_NORON    0x13
#define C_INVOFF   0x20
#define C_INVON    0x21
#define C_GAMSET   0x26
#define C_DISPOFF  0x28
#define C_DISPON   0x29
#define C_CASET    0x2A
#define C_RASET    0x2B
#define C_RAMWR    0x2C
#define C_CLRSPACE 0x2D
#define C_RAMRD    0x2E
#define C_PTLAR    0x30
#define C_VSCLLDEF 0x33
#define C_TEOFF    0x34
#define C_TEON     0x35
#define C_MADCTL   0x36
#define C_VSSTADRS 0x37
#define C_IDLEOF   0x38
#define C_IDLEON   0x39
#define C_COLMOD   0x3A
#define C_FRMCTR1  0xB1
#define C_FRMCTR2  0xB2
#define C_FRMCTR3  0xB3
#define C_INVCTR   0xB4
#define C_RGBBLK   0xB5
#define C_DISSET5  0xB6
#define C_SDRVDIR  0xB7
#define C_GDRVDIR  0xB8
#define C_PWCTR1   0xC0
#define C_PWCTR2   0xC1
#define C_PWCTR3   0xC2
#define C_PWCTR4   0xC3
#define C_PWCTR5   0xC4
#define C_VMCTR1   0xC5
#define C_VCOMCTR2 0xC6
#define C_VCOMOFFS 0xC7
#define C_RDID1    0xDA
#define C_RDID2    0xDB
#define C_RDID3    0xDC
#define C_RDID4    0xDD
#define C_GMCTRP1  0xE0
#define C_GMCTRN1  0xE1
#define C_GAMRSEL  0xF2
#define C_PWCTR6   0xFC

// SD card faces out the top, connector along the bottom in hardware orientation
// But for a conventional ~4:3 layout, with "HY-1.8 SPI" upright and the SD card facing right, we need to rotate that
#define HEIGHT 128
#define WIDTH  160

// Parameters for terminal emulation, which is still in portrait
#define TXTW    21
#define TXTH    20
#define TXTTABW  8

#define RGBTOBGR565(rgb) (((rgb & 0xF800) >> 11) | (rgb & 0x07E0) | ((rgb & 0x001F) << 11))

#define BLACK   0
#define RED     0xF800 // 0b1111100000000000
#define GREEN   0x07E0 // 0b0000011111100000
#define BLUE    0x001F // 0b0000000000011111
#define CYAN    ~RED
#define MAGENTA ~GREEN
#define YELLOW  ~BLUE
#define WHITE   ~BLACK

// spi.c
extern void lcdoutbn(const unsigned char *buf, unsigned int len);
extern void lcdoutwr(unsigned int w, unsigned int cnt);
extern void lcdreset(void);
extern void lcdsetcmd(void);
extern void lcdsetdat(void);
extern void lcdspibegin(void);
extern void lcdspiend(void);

// command.c
extern void lcdcmd(unsigned char cmd, const unsigned char *data, int datalen, int delay);
extern void lcdsetaddresswindow(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
extern void lcdinit(void);
extern void lcdsethscroll(int hs);

// graphics.c
extern void gfx_plot(int x, int y, int colour);
extern void gfx_line(int x0, int y0, int x1, int y1, int colour);
extern void gfx_box(int x0, int y0, int x1, int y1, int colour);
extern void gfx_boxfilled(int x0, int y0, int x1, int y1, int colour);
extern void gfx_clear(int colour);
extern void gfx_circle(int cx, int cy, int r, int colour);
extern void gfx_circlefilled(int cx, int cy, int r, int colour);

// font.c
extern unsigned char font_bin[];

// text.c
extern void gfx_textchartransparent(char c, int x, int y, int fgcol);
extern void gfx_textcharopaque(char c, int x, int y, int fgcol, int bgcol);

// term.c
extern void gfx_termoutc(unsigned char c);

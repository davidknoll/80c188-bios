SRCS=command.c font.c graphics.c sdcard.c spi.c term.c text.c
HDRS=hy18.h ..\c\ioports.h
CFLAGS=-1 -I..\c -I. -I..\fatfs

all: hy18_mc.lib hy18_mt.lib hellolib.hex hellosd.hex testcard.hex

clean:
	del *.obj *.lib font.c *.com *.map *.hex

font.c: font.bin
	srec_cat font.bin -Binary -Output font.c -C-Array font_bin

hy18_mc.lib: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -mc -c -ocommand_mc.obj command.c
	$(CC) $(CFLAGS) -mc -c -ofont_mc.obj font.c
	$(CC) $(CFLAGS) -mc -c -ographics_mc.obj graphics.c
	$(CC) $(CFLAGS) -mc -c -osdcard_mc.obj sdcard.c
	$(CC) $(CFLAGS) -mc -c -ospi_mc.obj spi.c
	$(CC) $(CFLAGS) -mc -c -oterm_mc.obj term.c
	$(CC) $(CFLAGS) -mc -c -otext_mc.obj text.c
	tlib $@ + command_mc.obj + font_mc.obj + graphics_mc.obj + sdcard_mc.obj + spi_mc.obj + term_mc.obj + text_mc.obj

hy18_mt.lib: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -mt -c -ocommand_mt.obj command.c
	$(CC) $(CFLAGS) -mt -c -ofont_mt.obj font.c
	$(CC) $(CFLAGS) -mt -c -ographics_mt.obj graphics.c
	$(CC) $(CFLAGS) -mt -c -osdcard_mt.obj sdcard.c
	$(CC) $(CFLAGS) -mt -c -ospi_mt.obj spi.c
	$(CC) $(CFLAGS) -mt -c -oterm_mt.obj term.c
	$(CC) $(CFLAGS) -mt -c -otext_mt.obj text.c
	tlib $@ + command_mt.obj + font_mt.obj + graphics_mt.obj + sdcard_mt.obj + spi_mt.obj + term_mt.obj + text_mt.obj

hellolib.com: ..\c\crt0ram.obj hellolib.obj hy18_mt.lib
	tlink /t /m ..\c\crt0ram.obj hellolib.obj, hellolib.com, hellolib.map, ct.lib hy18_mt.lib

hellosd.com: ..\c\crt0ram.obj hellosd.obj hy18_mt.lib
	tlink /t /m ..\c\crt0ram.obj hellosd.obj ..\c\iofunc.obj, hellosd.com, hellosd.map, ct.lib hy18_mt.lib

testcard.com: ..\c\crt0ram.obj testcard.obj hy18_mt.lib ..\fatfs\fatfs_mt.lib
	tlink /t /m ..\c\crt0ram.obj testcard.obj ..\c\iofunc.obj, testcard.com, testcard.map, ct.lib hy18_mt.lib ..\fatfs\fatfs_mt.lib

.com.hex:
	srec_cat -Output $@ -Intel --address-length=3 $< -Binary -offset 0x10100 -execution_start_address 0x10000100

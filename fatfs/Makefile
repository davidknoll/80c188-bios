SRCS=ff.c ffsystem.c ffunicode.c sdmm.c
HDRS=diskio.h ff.h ffconf.h integer.h ..\bios\bios.h ..\c\ioports.h
CFLAGS=-1 -I..\bios -I..\c

all: fatfs_mc.lib fatfs_mt.lib

clean:
	del *.obj *.lib

fatfs_mc.lib: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -mc -c -off_mc.obj ff.c
	$(CC) $(CFLAGS) -mc -c -offsystem_mc.obj ffsystem.c
	$(CC) $(CFLAGS) -mc -c -offunicode_mc.obj ffunicode.c
	$(CC) $(CFLAGS) -mc -c -osdmm_mc.obj sdmm.c
	tlib $@ + ff_mc.obj + ffsystem_mc.obj + ffunicode_mc.obj + sdmm_mc.obj

fatfs_mt.lib: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -mt -c -off_mt.obj ff.c
	$(CC) $(CFLAGS) -mt -c -offsystem_mt.obj ffsystem.c
	$(CC) $(CFLAGS) -mt -c -offunicode_mt.obj ffunicode.c
	$(CC) $(CFLAGS) -mt -c -osdmm_mt.obj sdmm.c
	tlib $@ + ff_mt.obj + ffsystem_mt.obj + ffunicode_mt.obj + sdmm_mt.obj

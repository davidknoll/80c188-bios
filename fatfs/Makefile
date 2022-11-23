SRCS=diskio.c ff.c ffsystem.c ffunicode.c
HDRS=diskio.h ff.h ffconf.h ..\libhy18\hy18.h
CFLAGS=-1 -I..\bios -I..\c -I..\libhy18

all: fatfs_mc.lib fatfs_mt.lib

clean:
	del *.obj *.lib

fatfs_mc.lib: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -mc -c -odiskio_mc.obj diskio.c
	$(CC) $(CFLAGS) -mc -c -off_mc.obj ff.c
	$(CC) $(CFLAGS) -mc -c -offsystem_mc.obj ffsystem.c
	$(CC) $(CFLAGS) -mc -c -offunicode_mc.obj ffunicode.c
	tlib $@ + diskio_mc.obj + ff_mc.obj + ffsystem_mc.obj + ffunicode_mc.obj

fatfs_mt.lib: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -mt -c -odiskio_mt.obj diskio.c
	$(CC) $(CFLAGS) -mt -c -off_mt.obj ff.c
	$(CC) $(CFLAGS) -mt -c -offsystem_mt.obj ffsystem.c
	$(CC) $(CFLAGS) -mt -c -offunicode_mt.obj ffunicode.c
	tlib $@ + diskio_mt.obj + ff_mt.obj + ffsystem_mt.obj + ffunicode_mt.obj

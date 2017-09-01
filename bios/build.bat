cls
bcc -1 -mc -I..\c -c int08h.c
bcc -1 -mc -I..\c -c int10h.c
bcc -1 -mc -I..\c -I..\c\fatfs -c int13h.c
bcc -1 -mc -I..\c -c int14h.c
bcc -1 -mc -I..\c -I..\c\fatfs -c int15h.c
bcc -1 -mc -I..\c -c int16h.c
bcc -1 -mc -I..\c -c int17h.c
bin2c -o patb.h -m -n patb ..\patb\PATB-MTM.com
bcc -1 -mc -I..\c -c int18h.c
bcc -1 -mc -I..\c -c int19h.c
bcc -1 -mc -I..\c -c int1Ah.c
bcc -1 -mc -I..\c -c intcpu.c
bcc -1 -mc -I..\c -DTESTING -c main.c
bcc -1 -mc -I..\c -DPOLLED -c serial.c

tasm /d__ROM__ /dINITSP=4000h ..\c\crt0ram.asm, crt0rom.obj
bcc -1 -mc -I..\c -DNOSERIOB -oiofunc.obj -c ..\c\iofunc.c
bcc -1 -mc -I..\c -c ddptfdpt.c

bcc -1 -mc -I..\c -I..\c\fatfs -off.obj -c ..\c\fatfs\ff.c
bcc -1 -mc -I..\c -I..\c\fatfs -osdmm.obj -c ..\c\fatfs\sdmm.c
bcc -1 -mc -I..\c -I..\c\fatfs -ounicode.obj -c ..\c\fatfs\ffunicode.c
bcc -1 -mc -I..\c -I..\c\fatfs -offsystem.obj -c ..\c\fatfs\ffsystem.c

tlink @bios.rsp
rem exe2bin bios.exe bios.bin
rem bin2hex bios.bin bios.hex

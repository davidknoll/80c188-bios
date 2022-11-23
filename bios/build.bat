cls
bcc -1 -mc -I..\c -c console.c
bcc -1 -mc -I..\c -c ddptfdpt.c
bcc -1 -mc -I..\c -c int08h.c
bcc -1 -mc -I..\c -c int10h.c
bcc -1 -mc -I..\c -I..\fatfs -c int13h.c
bcc -1 -mc -I..\c -c int14h.c
bcc -1 -mc -I..\c -I..\fatfs -c int15h.c
bcc -1 -mc -I..\c -c int16h.c
bcc -1 -mc -I..\c -c int17h.c
bcc -1 -mc -I..\c -c int18h.c
bcc -1 -mc -I..\c -c int19h.c
bcc -1 -mc -I..\c -c int1Ah.c
bcc -1 -mc -I..\c -c intcpu.c
bcc -1 -mc -I..\c -DTESTING -c main.c
bcc -1 -mc -I..\c -DPOLLED -c serial.c

srec_cat ..\patb\PATB-MTM.com -Binary -Output patb.inc -ASM
tasm patb.asm, patb.obj
tasm /d__ROM__ /dINITSP=4000h ..\c\crt0ram.asm, crt0rom.obj

pushd ..\fatfs
make fatfs_mc.lib
popd

pushd ..\libhy18
make hy18_mc.lib
popd

tlink @bios.rsp
rem exe2rom bios.exe bios.bin 7000 7XXX 7C00
rem srec_cat -Output bios.hex -Intel --address-length=3 bios.bin -Binary -offset 0x70000 -execution_start_address 0x70000000

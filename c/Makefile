CFLAGS=-1 -mt -I..\fatfs

all: hello.hex fftest.hex

clean:
	del hello.hex hello.com hello.map
	del fftest.hex fftest.com fftest.map
	del crt0ram.obj hello.obj fftest.obj iofunc.obj

hello.com: crt0ram.obj hello.obj iofunc.obj
	tlink /t /m crt0ram.obj hello.obj iofunc.obj, hello.com, hello.map, ct.lib

fftest.com: crt0ram.obj fftest.obj iofunc.obj ..\fatfs\fatfs_mt.lib
	tlink @fftest.rsp

crt0rom.obj: crt0ram.asm
	tasm /d__ROM__ crt0ram.asm, crt0rom.obj

..\fatfs\fatfs_mt.lib:
	pushd ..\fatfs
	make fatfs_mt.lib
	popd

.com.hex:
	srec_cat -Output $@ -Intel --address-length=3 $< -Binary -offset 0x10100 -execution_start_address 0x10000100

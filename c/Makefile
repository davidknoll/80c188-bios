CFLAGS=-1 -mt -Ifatfs

all: hello.hex fftest.hex

clean:
	del hello.hex hello.com hello.map
	del fftest.hex fftest.com fftest.map
	del crt0ram.obj hello.obj fftest.obj iofunc.obj

hello.com: crt0ram.obj hello.obj iofunc.obj
	tlink /t /m crt0ram.obj hello.obj iofunc.obj, hello.com, hello.map, ct.lib

fftest.com: crt0ram.obj fftest.obj iofunc.obj
	tlink @fftest.rsp

crt0rom.obj: crt0ram.asm
	tasm /d__ROM__ crt0ram.asm, crt0rom.obj

.com.hex:
	bin2hex $< $@ -s 1000 -o 0100 -e 0100

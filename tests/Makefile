all: beep.hex hello.hex rtctest.hex flashtst.hex

clean:
	del *.hex *.lst

.asm.hex:
	nasm -I../ -o $@ -f ith -l $*.lst $<

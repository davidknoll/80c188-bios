; Basic test of RTC tick for David's new 80C188
; Assemble with:
; nasm -o rtctest.srec -f srec -l rtctest.lst rtctest.asm
			[list -]
			%include "include/ioports.inc"
			%include "include/sndmacro.inc"
			[list +]

			section .text start=10000h vstart=0f0000h
begin:		outp rtc_ctla, 20h		; Control A
			outp rtc_ctlb, 82h		; Control B
			outp rtc_sec, 00h		; Seconds
			outp rtc_ctlb, 02h		; Control B

.1			mov bl, al
.2			mov dx, rtc_sec			; Repeatedly check the seconds field
			in al, dx
			cmp al, bl				; Waiting until it changes
			jz .1

			mov bl, al
			note NC4, 1				; Then beep
			jmp .2

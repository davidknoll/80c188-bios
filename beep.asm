; Beep, call/return, UART bus test program for David's new 80C188
; Assemble with:
; nasm -o beep.srec -f srec -l beep.lst beep.asm
			[list -]
			%include "include/ioports.inc"
			%include "include/sndmacro.inc"
			[list +]

			section .text start=10000h, vstart=0f0000h
begin:		outp uart_mcr, 0fh	; Set /DTR, /RTS, /OUT1, /OUT2 low
			call tune
			call del4

			outp uart_mcr, 00h	; Set /DTR, /RTS, /OUT1, /OUT2 high
			call tune
			call del4
			jmp begin

tune:		note NC4, 1			; Play a tune
			note NC4, 1
			note NC4, 1
			note ND4, 1
			note NE4, 2
			note ND4, 2
			note NC4, 1
			note NE4, 1
			note ND4, 1
			note ND4, 1
			note NC4, 1
			ret

del4:		delay				; A short delay
			delay
			delay
			delay
			ret

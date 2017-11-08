; Hello World serial port test for David's new 80C188
; Assemble with:
; nasm -o hello.hex -f ith -l hello.lst hello.asm
			[list -]
			%include "include/ioports.inc"
			[list +]

;			section .text start=10000h, vstart=0f0000h
			section .text start=0100h
begin:		cld
			outp uart_lcr, 83h	; Enable DLAB
			outp uart_dll, uartdll(115200)	; Baud rate divisor
			outp uart_dlm, uartdlm(115200)
			outp uart_lcr, 03h	; Disable DLAB, set 8N1

			mov ax, cs			; Output message
			mov ds, ax
.1			mov si, tx_hello
			call outstr
			jmp .1

; Output byte to serial port
; Entry: AL = byte to output, DX changed
seroutb:	push ax
			mov dx, uart_lsr	; Wait for THRE
.1			in al, dx
			test al, 1<<5
			jz .1
			pop ax
			mov dx, uart_thr	; Output the byte
			out dx, al
			ret

; Output string to serial port
; Entry: DS:SI points to string
; Exit: DS:SI points to byte following terminating null, AX changed
outstr:		lodsb				; Get byte, increment pointer
			and al, al			; Terminating null?
			jz .1
			call seroutb		; Output byte
			jmp outstr			; Back for more
.1			ret

tx_hello:	db "Hello, World!", 0Dh, 0Ah, 00h

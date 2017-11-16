; Flash ROM write test for David's new 80C188
; Assemble with:
; nasm -o flashtst.hex -f ith -l flashtst.lst flashtst.asm
			[list -]
			%include "include/ioports.inc"
			[list +]

			section .text start=0100h
begin:		cli
			cld
			outp uart_lcr, 83h	; Enable DLAB
			outp uart_dll, uartdll(115200)	; Baud rate divisor
			outp uart_dlm, uartdlm(115200)
			outp uart_lcr, 03h	; Disable DLAB, set 8N1

			mov ax, 0E000h		; Set destination
			mov es, ax
			xor ax, ax
			mov di, ax
			mov cx, 0080h		; Set count

			mov [es:5555h], byte 0AAh	; Relax SDP
			mov [es:2AAAh], byte 055h
			mov [es:5555h], byte 0A0h

.1			mov al, cl			; Write a sector's worth
			stosb
			loop .1

			mov bl, [es:0000h]	; Toggle bit polling
.2			mov al, bl
			mov bl, [es:0000h]
			xor al, bl
			test al, 1<<6
			jnz .2

			mov al, '*'			; Finished
			call seroutb
			hlt

; Output byte to serial port
; Entry: AL = byte to output
; Exit: DX changed
seroutb:	push ax
			mov dx, uart_lsr	; Wait for THRE
.1			in al, dx
			test al, 1<<5
			jz .1
			pop ax
			mov dx, uart_thr	; Output the byte
			out dx, al
			ret

; Intel hex loader for David's new 80C188
; Checksums are ignored. Load SS:SP before starting.
; Assemble with:
; nasm -o hexload.hex -f ith -l hexload.lst hexload.asm
			[list -]
			%include "include/ioports.inc"
			[list +]

			section startup start=10000h vstart=0F0000h
hexload:	cli
			cld
			outp uart_lcr, 83h				; Enable DLAB
			outp uart_dll, uartdll(38400)	; Baud rate divisor
			outp uart_dlm, uartdlm(38400)
			outp uart_lcr, 03h				; Disable DLAB, set 8N1

			mov ax, cs						; Output message
			mov ds, ax
			mov si, tx_hello
			call outstr
			mov ax, 0050h					; Default to load segment 0050h
			mov es, ax

.colon		call serinb						; Start of record
			cmp al, ':'
			jnz .colon
			call serinhb					; Byte count
			xor cx, cx
			mov cl, al
			call serinhw					; Address
			mov di, ax

			call serinhb					; Record type
			and al, al						; 00 (data)
			jz .data
			dec al							; 01 (EOF / start address on 8-bit)
			jz .eof8
			dec al							; 02 (segment address on 16-bit)
			jz .seg16
			dec al							; 03 (segmented start address on 16-bit)
			jz .eof16
			dec al							; 04 (high word of linear address on 32-bit)
			jz .seg32
			dec al							; 05 (linear start address on 32-bit)
			jz .eof32

.bad		mov al, '?'						; Bad or unrecognised record
			call seroutb
			jmp .colon

.data		and cl, cl						; If it's a data record it must have some data
			jz .bad
.data1		call serinhb					; Receive and store data bytes
			stosb
			loop .data1

.cksum		call serinhb					; Get checksum (but ignore it for now)
			mov al, '.'						; Good record
			call seroutb
			jmp .colon						; Go back for next record

.eof8		and cl, cl						; Must have empty data field
			jnz .bad
			and di, di						; If address field is zero, assume it's just an EOF, not a start address, and ignore it
			jz .cksum

.cksumeof	call serinhb					; Get checksum (but ignore it for now)
			mov al, '*'						; Good record with start address
			call seroutb
			mov al, 0Dh
			call seroutb
			mov al, 0Ah
			call seroutb
			call seroutb
			push es							; Push what will be the starting CS:IP
			push di
			push es							; Set DS to same as CS and ES
			pop ds
			retf							; Jump to the new CS:IP

.seg16		cmp cl, 02h
			jnz .bad
			call serinhw					; Get new load segment
			mov es, ax
			jmp .cksum

.eof16		cmp cl, 04h
			jnz .bad
			call serinhw					; Get starting CS
			mov es, ax
			call serinhw					; Get starting IP
			mov di, ax
			jmp .cksumeof

.seg32		cmp cl, 02h
			jnz .bad
			call serinhw					; Get new high word of load address
			shl ax, 12						; Can only use its lowest nibble though
			mov es, ax
			jmp .cksum

.eof32		cmp cl, 04h
			jnz .bad
			call serinhw					; Get high word of start address
			shl ax, 12						; Can only use its lowest nibble though
			mov es, ax
			call serinhw					; Get low word of start address
			mov di, ax
			jmp .cksumeof

; Input hex word/byte/nibble
serinhw:	call serinhb
			mov ah, al
			call serinhb
			ret

serinhb:	call serinhn
			shl al, 4
			mov bl, al
			call serinhn
			or al, bl
			ret

serinhn:	call serinb
			cmp al, '0'
			jb serinhn
			cmp al, '9'
			jbe .1
			cmp al, 'A'
			jb serinhn
			cmp al, 'F'
			ja serinhn
			sub al, 'A'-0Ah
			ret
.1			sub al, '0'
			ret

; Input byte from serial port, blocking
; Exit: AL = byte received, DX changed
serinb:		mov dx, uart_lsr				; Wait for DR
.1			in al, dx
			test al, 1 << 0
			jz .1
			mov dx, uart_rbr				; Input the byte
			in al, dx
			ret

; Output byte to serial port
; Entry: AL = byte to output
; Exit: DX changed
seroutb:	push ax
			mov dx, uart_lsr				; Wait for THRE
.1			in al, dx
			test al, 1 << 5
			jz .1
			pop ax
			mov dx, uart_thr				; Output the byte
			out dx, al
			ret

; Output string to serial port
; Entry: DS:SI points to string
; Exit: DS:SI points to byte following terminating null, AX changed
outstr:		lodsb							; Get byte, increment pointer
			and al, al						; Terminating null?
			jz .1
			call seroutb					; Output byte
			jmp outstr						; Back for more
.1			ret

; Clear screen, sign-on message
tx_hello	db 0Fh, 1Bh, "[H", 1Bh, "[2J", 0Dh, 0Ah
			db "Intel hex loader for David's 80C188 SBC", 0Dh, 0Ah, 00h

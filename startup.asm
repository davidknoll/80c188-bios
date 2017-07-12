; ROM startup code for David's new 80C188
; This is based on the STARTUP.ASM for the N8VEM SBC-188, which is under GPL 3.
; Assemble with:
; nasm -o startup.hex -f ith -l startup.lst startup.asm
			[list -]
			%include "include/ioports.inc"
			%include "include/sndmacro.inc"
			[list +]

			section startup start=1FF00h vstart=0FFF00h
begin:		cli
			cld
			mov dh, ip_base >> 8
			mov si, table			; Point to the table
			mov ax, cs
			mov ds, ax
			mov cx, tablecnt

.1			lodsb					; Initialise registers from table
			mov dl, al
			lodsw
			out dx, ax
			loop .1

			note NC4, 1				; Startup tones
			note ND4, 1
			note NE4, 2

			mov ax, 7000h			; Stack at top of RAM
			mov ss, ax
			mov sp, 0000h
			mov ax, 0F000h			; Set CS=DS=ES
			mov ds, ax
			mov es, ax
			jmp 0F000h:0000h		; Continue to body of ROM

table		db_lo ics_umcs			; ROM
			dw 0E03Dh				; 128KB, 1 wait state, no external ready
			db_lo ics_mmcs			; RAM
			dw 01FCh				; 00000h, no wait states, no external ready
			db_lo ics_mpcs			; /MCSx size, /PCSx configuration
			dw 0C0B9h				; 512KB, 7 /PCSx, I/O space, 1 wait state, external ready
			db_lo ics_pacs			; External peripherals
			dw 0FB9h				; F800h, 1 wait state, external ready
tablecnt	equ ($-table)/3

			setloc 0F0h				; Reset entry is FFFF:0000h
			jmp 0F000h:begin		; Jump to the startup code above
			db "02/04/14", 00h		; BIOS date (mm/dd/yy)
			db 0FEh, 0FFh			; Model identifier (FEh is a 1982 XT)

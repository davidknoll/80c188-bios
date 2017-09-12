; Minimal startup code for Borland C++ on David's new 80C188
; This is based on C:\BC5\LIB\STARTUP\C0.ASM from Borland C++.
; Not everything is implemented. In particular, stdio won't work
; because we're using Borland's 16-bit DOS libc without DOS.
; Treatment of constructors/destructors and atexits is not implemented.
; Using tiny memory model in RAM, something with far data in ROM.
; Compile with something like:
;   bcc -1 -mt -c crt0ram.asm
;   bcc -1 -mt -c hello.c
;   tlink /t /m crt0ram.obj hello.obj, hello.com, hello.map, ct.lib
; To convert this to load in MON88:
;   objcopy -I binary -O ihex --change-addresses 0x100 HELLO.COM hello.hex
; Then insert/remove type 02/03 records if required.

			.186

IFDEF __ROM__
			ASSUME CS:CGROUP, DS:DGROUP
			GROUP CGROUP _TEXT, _TXTEND
			GROUP DGROUP _DATA, _CVTSEG, _BSS, _BSSEND
ELSE
			ASSUME CS:DGROUP, DS:DGROUP
			GROUP DGROUP _TEXT, _TXTEND, _DATA, _CVTSEG, _BSS, _BSSEND
ENDIF

			EXTRN _main:NEAR, _exit:NEAR
			PUBLIC __start
IFDEF __ROM__
			PUBLIC _callfards
ENDIF
			PUBLIC __terminate, __checknull, __restorezero, __cleanup
			PUBLIC _errno, __heapbase, __brklvl, __heaptop, __psp
			PUBLIC __version, __realcvtvector
			PUBLIC DGROUP@

_TEXT		SEGMENT BYTE PUBLIC 'CODE'
IFNDEF __ROM__
			ORG 100h					; For COM file compatibility
ENDIF
__start		PROC NEAR
; Initialise segment registers
			cli							; As we're changing the SS/SP
IFDEF __ROM__
			mov ax, cs:DGROUP@
ELSE
			mov ax, cs
			mov cs:DGROUP@, ax			; Save segment for later restoration
ENDIF
			mov ds, ax
			mov es, ax
			mov ss, ax
IFDEF INITSP
			mov sp, INITSP
ELSE
			mov sp, 0000h
ENDIF
			sti

; Copy initialised data into place
IFDEF __ROM__
			mov ax, offset CGROUP:etext@	; Assume data is right after code in the binary
			shr ax, 4					; and calculate a segment selector for it
			mov si, cs
			add ax, si
			mov ds, ax
			xor si, si
			xor di, di
			mov cx, offset DGROUP:edata@	; Bytes to copy
			cld
			rep movsb
			mov ds, cs:DGROUP@
ENDIF

; Zero the BSS
			xor ax, ax
			mov es, cs:DGROUP@
			mov di, offset DGROUP:edata@
			mov cx, offset DGROUP:ebss@
			sub cx, di
			cld
			rep stosb

; Call main(), and if that returns, exit() with the return code
IFDEF __ROM__
			push DGROUP
ENDIF
			push offset DGROUP:envp		; char *envp[]
IFDEF __ROM__
			push DGROUP
ENDIF
			push offset DGROUP:argv		; char *argv[]
			push 1						; int argc
			call _main
			add sp, 6

			push ax						; int status
			call _exit
			pop ax

__terminate	LABEL NEAR
			cli							; Halt the system, there's no OS to return to
			hlt
			jmp __terminate
__checknull	LABEL NEAR
__restorezero LABEL NEAR
__cleanup	LABEL NEAR
			ret
			ENDP

; void callfards(void (far *funcptr)(void))
IFDEF __ROM__
_callfards	PROC NEAR
			push bp
			mov	bp, sp
			mov ds, [bp+6]
			call dword ptr [bp+4]
			pop bp
			ret
			ENDP
ENDIF

IFDEF __ROM__
DGROUP@		dw DGROUP					; In ROM, we can't write here
ELSE
DGROUP@		dw ?
ENDIF
			ENDS
_TXTEND		SEGMENT PARA PUBLIC 'TXTEND'
etext@		LABEL BYTE
			ENDS

_DATA		SEGMENT PARA PUBLIC 'DATA'
argv0		db 'C', 00h					; Minimal arguments to main()
argv		dw offset DGROUP:argv0
IFDEF __ROM__
			dw DGROUP
ENDIF
envp		dw 0000h					; Null pointer for empty environment
IFDEF __ROM__
			dw 0000h
ENDIF

_errno		dw 0000h
__heapbase	dw offset DGROUP:ebss@
IFDEF __ROM__
			dw DGROUP
ENDIF
__brklvl	dw offset DGROUP:ebss@
IFDEF __ROM__
			dw DGROUP
ENDIF
__heaptop	dw offset DGROUP:ebss@
IFDEF __ROM__
			dw DGROUP
ENDIF
__psp		dw 0000h
__version	equ 0000h
			ENDS
_CVTSEG		SEGMENT WORD PUBLIC 'DATA'
__realcvtvector LABEL WORD
edata@		LABEL BYTE
			ENDS

_BSS		SEGMENT WORD PUBLIC 'BSS'
			ENDS
_BSSEND		SEGMENT BYTE PUBLIC 'BSSEND'
ebss@		LABEL BYTE
			ENDS
; After the BSS, are the heap (growing upwards)
; and the stack (growing downwards)

IFDEF __ROM__
			PUBLIC _IVT, _BDA
; Interrupt vector table
_IVTSEG		SEGMENT AT 0000h
_IVT		LABEL DWORD
			dd 256 DUP(?)
			ENDS

; BIOS data area
_BDASEG		SEGMENT AT 0040h
_BDA		LABEL BYTE
			db 256 DUP(?)
			ENDS
ENDIF

			END __start

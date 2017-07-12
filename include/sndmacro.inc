; Macros for creating basic beeps

; Musical note frequencies
NC4			equ 262
ND4			equ 294
NE4			equ 330

; Timers are clocked at 1/4 of CPU = 1/8 of xtal
; For 50% duty cycle, need MCA = MCB = 1/2 of period
%macro soundhz 1
			mov dx, it1_mca		; Set max count A
			mov ax, (F_CPU/8)/%1
			out dx, ax
			inc dx				; Set max count B to the same value
			inc dx
			out dx, ax
			inc dx				; Set control word
			inc dx
			mov ax, 0C003h		; Start the timer
			out dx, ax
%endmacro

%macro silence 0
			mov dx, it1_cw
			mov ax, 4003h		; Stop the timer
			out dx, ax
%endmacro

%macro delay 0
			xor cx, cx
			loop $
%endmacro

%macro note 2
			soundhz %1
%rep %2
			delay
%endrep
			silence
			delay
%endmacro

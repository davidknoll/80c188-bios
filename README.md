David's 80C188 SBC BIOS
=======================

This package contains a BIOS for my hobby project 80C188 SBC,
and some associated tools and test code.

To build the BIOS, you will currently need to be running 32-bit Windows,
and have installed/in your PATH (in addition to a bunch of manual editing):
* Borland C++ (bcc)
* Borland Turbo Assembler (tasm)
  * These are not free software, but are available on some abandonware sites.
I should probably convert things to use Open Watcom instead.
* `srecord`, specifically the `srec_cat` utility
  * To embed Palo Alto Tiny BASIC as ROM BASIC
  * GPL3+, available from http://srecord.sourceforge.net/
* `exe2rom` (included)
  * To convert the linked EXE file to a raw binary, relocated for ROM use
  * By me, specially for this project
* Netwide Assembler (nasm)
  * For the 80C188 startup code
  * 2-clause BSD, available from http://www.nasm.us/

Some configuration details are in bios/main.c and c/ioports.h,
as well as the code being pretty specific to the hardware.

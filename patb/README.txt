Here is an explanation of the files in this compressed folder.

PATB.JPG is a photo of the cover of the Interface Age Book
PATBS-SM.JPG is the same photo in a smaller format.

PATB.PDF is a scan of the original source code for Palo Alto
Tiny Basic, from the Interface Age book.

PATB-MTM.ASM is the source code for the MTM version of PATB.
The source code will compile with NASM.  Note that no DOS
interrupts are used in the code.

PATB-MTM.COM is a compiled version of the MTM version of PATB.
This version will run on any PC compatible computer. A good way
to start playing with the program...

PATB-MAN.TXT is a short manual about how to use PATB.  Note
that the main working difference is how to SAVE and LOAD files.
Note: This would really benefit from a good file system, and
coding modificatons to work with larger files. 

PATB-ROM.BIN is a binary file, that if burned and placed in
IBM PC socket U-29 will boot PATB instead of CASSETTE BASIC.
This code will fit in a 27128 EPROM without modification, by
using an adapter: http://www.minuszerodegrees.net/5150_u33.htm 

RELOC.TXT is the source code for the relocation code in the ROM.
This was compiled using DEBUG. Simply change source and destination
addresses as your application warrants.  

README.TXT is this file.

This code has not been extensively error checked.  I would appreciate
any and all help to improve the project.  Thank You!

Michael 
MTM Scientific, Inc (www.mtmscientific.com)
EMAIL: mtm at mtmscientific dot com
11/2013
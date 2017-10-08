Ideas
=====

Hardware
* What to put on an expansion besides more RAM? IDE? CRTC/terminal?
* Where are those other 512KB SRAM chips, a DS1250 is unnecessary
* Change baud rate xtal to more standard 1.8432MHz

Software
* Count/detect available memory, anticipating expansion. (Impact on data segment?)
* Expand Int 15h with implementing more "standard" misc functions
* Kermit/X/Y/ZMODEM downloader, they retry on error unlike a basic hex loader
* For that matter, what other utilities could be included?
* And let's have a hotkey to interrupt normal boot sequence
* DOS on boot thinks time is 12:00, but can set RTC, date is read OK (think tick count?)
* Test the oprom functionality, for that matter
* Flash updating the ROM in-system
* Test all interrupts (think the RTC one was dodgy), use them
* Buffer TX too? pushcli/popcli when accessing buffer? Automatic treatment of RTS/CTS?
* RODATA section within CGROUP, to hold things like the PATB image and standard DDPTs
* Make use of 80C188's DMA, eg for memcpy, flashing, serial/hex loading
* Rearrange directory structure, build with makefiles, put eg FatFs in a library

Done-ish
* The terminal doesn't clear when I type CLS in DOS, or start FDISK- text is just overlayed
  * It does clear now, after I extended Int 10h / AH = 00h. But scrolling goes weird- 24 vs 25 lines?
* BIOS's own console I/O should use Int 10h/16h, so other devices could substitute their oproms
  * Done but still need to test oproms, and do C0000h early as a special case for video
* Detect checksum failures in hex/srec loader (Int 18h and hexload.asm)
  * Are indicated with a ! but loading will continue and valid exec addresses will still be taken
* Count/detect COM & LPT ports.

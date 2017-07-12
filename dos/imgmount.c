#include <stdio.h>

unsigned char eject(unsigned char drive)
{
	unsigned char rtn;
	asm {
		mov ah, 0x62
		mov dl, drive
		int 13h
		mov rtn, al
	}
	return rtn;
}

unsigned char insert(unsigned char drive, const char far *imgfile, unsigned char mode)
{
	unsigned char rtn;
	asm {
		mov ah, 0x61
		mov al, mode
		mov dl, drive
		mov bx, imgfile+2
		mov es, bx
		mov bx, imgfile
		int 13h
		mov rtn, al
	}
	return rtn;
}

int main(int argc, char *argv[])
{
	unsigned char drive = toupper(argv[2][0]) - 'A';
	unsigned char mode;
	unsigned char rtn;
	if (argc < 3 || argc > 4 || drive >= 2) goto usage;

	if (!strcmp("/ro", argv[3])) { mode = 0x00; }
	else if (!strcmp("/rw", argv[3])) { mode = 0x01; }
	else if (!strcmp("/rwn", argv[3])) { mode = 0x02; }
	else if (!strcmp("/rwc", argv[3])) { mode = 0x03; }
	else if (argc == 4) { goto usage; }
	else { mode = 0x01; }

	rtn = eject(drive);
	if (rtn) {
		fprintf(stderr, "Drive %c empty or failed to eject with error %02Xh.\n", drive + 'A', rtn);
	} else {
		printf("Disk ejected from drive %c.\n", drive + 'A');
	}

	if (strcmp("/e", argv[1])) {
		rtn = insert(drive, argv[1], 0x02);
		if (rtn) {
			fprintf(stderr, "Failed to mount %s on drive %c with error %02Xh.\n", argv[1], drive + 'A', rtn);
		} else {
			printf("Image %s successfully mounted on drive %c.\n", argv[1], drive + 'A');
		}
	}

	return rtn;

usage:
	fprintf(stderr, "Usage: %s imgfile drive:\n", argv[0]);
	return 1;
}

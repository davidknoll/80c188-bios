/*
 * EXE2ROM: Relocate an MZ-format EXE file for use in ROM
 * David Knoll
 *
 * We assume a read-only code segment, which will run from ROM,
 * and a writable data segment, which will follow the code in ROM
 * (as it does in the EXE file) but be copied to RAM by the startup code.
 *
 * The BSS follows the initialised data in RAM,
 * and should be zeroed by the startup code.
 *
 * Segment registers must be initialised by the startup code.
 * The boundary between code and data must be a paragraph boundary.
 * Pay attention to the memory model used.
 *
 * Usage: exe2rom exe-file rom-file code-seg code-len data-seg
 */
#include <stdio.h>
#include <stdlib.h>
#define bufw(off)		((unsigned short *) (buf + off))
#define reloctblw(off)	((unsigned short *) (reloctbl + off))
#define dataptrw(off)	((unsigned short *) (dataptr + off))

int main(int argc, char *argv[])
{
	unsigned char *buf, *reloctbl, *dataptr;
	unsigned short clseg, dlseg, entry;
	unsigned long cseglen, dataszb;
	FILE *fp;
	printf(
"EXE2ROM: Relocate an MZ-format EXE file for use in ROM\n\
David Knoll\n\n"
	);

	if (argc != 6) {
		fprintf(stderr,
"Usage: %s exe-file rom-file code-seg code-len data-seg\n\n\
  exe-file  Specifies the EXE file to be relocated\n\
  rom-file  Specifies the ROM image file to be created\n\
  code-seg  Segment where the code will live in ROM (hex)\n\
  code-len  Length of code segment in bytes (hex)\n\
  data-seg  Segment where the data will live once copied to RAM (hex)\n",
		argv[0]);
		goto err3;
	}
	clseg = strtoul(argv[3], NULL, 16);
	if (!clseg) {
		fprintf(stderr, "Invalid code segment specified.\n");
		goto err3;
	}
	cseglen = strtoul(argv[4], NULL, 16);
	if (!cseglen || cseglen % 16) {
		fprintf(stderr, "Invalid code segment length specified.\n");
		goto err3;
	}
	dlseg = strtoul(argv[5], NULL, 16);
	if (!dlseg) {
		fprintf(stderr, "Invalid data segment specified.\n");
		goto err3;
	}
	printf(
"EXE file: %s\n\
ROM file: %s\n\
Code segment: %04Xh Length: %04lXh\n\
Data segment: %04Xh\n",
	argv[1], argv[2], clseg, cseglen, dlseg);

	buf = malloc(1048576UL);	// More space than we need, but whatever
	if (!buf) {
		fprintf(stderr, "Unable to allocate memory.\n");
		goto err3;
	}
	// Read the EXE file
	fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "Unable to open input file.\n");
		goto err2;
	}
	if (fread(buf, 1, 1048576UL, fp) < 0x20) {
		fprintf(stderr, "Unable to read sufficient data from input file.\n");
		goto err1;
	}
	fclose(fp);

	if (*bufw(0x00) != 0x5A4D) {	// Check for MZ signature
		fprintf(stderr, "Input file is not an MZ-format EXE file.\n");
		goto err2;
	}
	reloctbl = buf + *bufw(0x18);			// Point to relocation table
	dataptr = buf + (*bufw(0x08) * 16UL);	// Point to payload
	// EXE payload size =
	//   (blocks in file * bytes/block)
	// - (512 - bytes in last block)
	// - (paragraphs in header * bytes/paragraph)
	dataszb = *bufw(0x04) * 512UL;
	if (*bufw(0x02)) dataszb -= 512 - *bufw(0x02);
	dataszb -= *bufw(0x08) * 16UL;
	if (!dataszb) {
		fprintf(stderr, "EXE file contains no payload.\n");
		goto err2;
	}
	if (!*bufw(0x06)) {
		fprintf(stderr, "EXE file contains no relocations. Is it tiny model?\n");
		goto err2;
	}
	printf("EXE payload size: %lXh Relocations: %d\n", dataszb, *bufw(0x06));

	for (entry = 0; entry < *bufw(0x06); entry++) {	// Count relocation entries
		// Offset to the reference to be relocated
		unsigned long curoff = *reloctblw(entry*4) + (*reloctblw((entry*4)+2) * 16UL);
		// Where that reference points
		unsigned short toreloc = *dataptrw(curoff);

		if (toreloc < cseglen / 16) {
			// The reference to be relocated is to the code segment
			toreloc += clseg;
		} else {
			// In RAM, there won't be the code segment before the data
			// segment like there is in the EXE file and the ROM binary
			toreloc -= cseglen / 16;
			// The reference to be relocated is to the data segment
			toreloc += dlseg;
		}
		*dataptrw(curoff) = toreloc;
	}

	// Write ROM file
	fp = fopen(argv[2], "wb");
	if (!fp) {
		fprintf(stderr, "Unable to open output file.\n");
		goto err2;
	}
	if (fwrite(dataptr, 1, dataszb, fp) != dataszb) {
		fprintf(stderr, "Unable to write output file.\n");
		goto err1;
	}
	fclose(fp);
	free(buf);
	printf("Conversion complete.\n");
	return EXIT_SUCCESS;

err1:
	fclose(fp);
err2:
	free(buf);
err3:
	return EXIT_FAILURE;
}

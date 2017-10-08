/* CPU flags */
#define F_C (1<<0)	/* Carry */
#define F_P (1<<2)	/* Parity */
#define F_A (1<<4)	/* Auxiliary (half) carry */
#define F_Z (1<<6)	/* Zero */
#define F_S (1<<7)	/* Sign */
#define F_T (1<<8)	/* Trace (single-step) */
#define F_I (1<<9)	/* Interrupt enable */
#define F_D (1<<10)	/* Direction */
#define F_O (1<<11)	/* Overflow */

/* Simple assembler instructions */
#define sti() asm sti
#define cli() asm cli
#define nop() asm nop
#define hlt() asm hlt

/* Enable access to the preserved registers inside an interrupt
 * handler, in order to retrieve arguments and return results for
 * eg. BIOS functions.
 * Highly dependent on the compiler-generated preamble.
 * Do something like: void interrupt int14h(struct pregs r)
 *
 * NB: SS and DS should be equal / are assumed to be equal in
 * memory models with near data pointers (tiny, small, medium), but the
 * interrupt handler preamble does not restore SS like it does DS.
 * So, in an environment where SS may have been changed (eg. other
 * programs are running), we want to write an interrupt handler, but we
 * don't want to bother with proper stack switching, it might be better
 * to use a far data model (compact, large, huge). Also, when SS is
 * somewhere that belongs to another program, we rely on that segment
 * being big enough for our use.
 */
struct pregs {
	/* Preserved by compiler-generated function preamble */
	unsigned int bp;
	unsigned int di;
	unsigned int si;
	unsigned int ds;
	unsigned int es;
	unsigned int dx;
	unsigned int cx;
	unsigned int bx;
	unsigned int ax;
	/* Preserved by the interrupt mechanism */
	unsigned int ip;
	unsigned int cs;
	unsigned int flags;
};

/* Diskette Drive Parameter Table */
struct ddpt {
	unsigned char stepunld;		/* Bits 7-4: step rate, bits 3-0: head unload time */
	unsigned char lddma;		/* Bits 7-1: head load time, bit 0: non-DMA mode */
	unsigned char mtroff;		/* Motor off time in clock ticks */
	unsigned char bps;			/* (128 << this) = bytes per sector */
	unsigned char spt;			/* Sectors per track */
	unsigned char gaplen;		/* Gap between sectors */
	unsigned char datalen;		/* Data length, ignored if bytes per sector field nonzero */
	unsigned char fmtgaplen;	/* Gap length when formatting */
	unsigned char fmtfill;		/* Format filler byte */
	unsigned char hdsetl;		/* Head settle time in ms */
	unsigned char mtrstart;		/* Motor start time in 1/8s */
	/* IBM SurePath BIOS */
	unsigned char maxcyl;		/* Maximum cylinder number */
	unsigned char datarate;		/* Data transfer rate */
	unsigned char cmostype;		/* Drive type in CMOS */
};

/* Fixed Disk Parameter Table */
struct fdpt {
	unsigned int cyl;			/* Number of cylinders */
	unsigned char hd;			/* Number of heads */
	unsigned int rwc;			/* Reduced write current start cylinder (XT only) */
	unsigned int precomp;		/* Write precompensation start cylinder (obsolete) */
	unsigned char eccbl;		/* Maximum ECC burst length (XT only) */
	unsigned char ctl;			/* Drive control byte */
	unsigned char stdtime;		/* Standard timeout (XT only) */
	unsigned char fmttime;		/* Formatting timeout (XT/WD1002 only) */
	unsigned char chktime;		/* Timeout for checking drive (XT/WD1002 only) */
	unsigned int lzone;			/* Landing zone cylinder (AT & later only) (obsolete) */
	unsigned char spt;			/* Sectors per track (AT & later only) */
	unsigned char reserved;		/* Reserved */
};

union farptr {
	unsigned char far *charptr;
	unsigned int far *intptr;
	void (far *funcptr)(void);
	struct {
		unsigned int off;
		unsigned int seg;
	} segoff;
};

extern void callfards(void (far *funcptr)(void));

/* These need to be explicitly declared far,
 * or it still gets assumed that they're in DGROUP
 */
extern void interrupt (* far IVT[256])();
extern volatile unsigned char far BDA[256];

extern void interrupt intnul(void);
extern void interrupt intunh();

/* CPU */
extern void interrupt int00h();	/* Divide */
extern void interrupt int01h();	/* Single step */
extern void interrupt int02h();	/* NMI */
extern void interrupt int03h();	/* Breakpoint */
extern void interrupt int04h();	/* INTO */
extern void interrupt int05h();	/* BOUND */
extern void interrupt int06h();	/* Invalid opcode */
extern void interrupt int07h();	/* ESC */

/* Hardware */
extern void interrupt int08h(); /* Timer 0 */
extern void interrupt int09h();	/* Reserved */
extern void interrupt int0Ah();	/* DMA 0 */
extern void interrupt int0Bh();	/* DMA 1 */
extern void interrupt int0Ch();	/* INT0 (RTC) */
extern void interrupt int0Dh();	/* INT1 (UART) */
extern void interrupt int0Eh();	/* INT2 (PPI) */
extern void interrupt int0Fh();	/* INT3 (expansion) */

/* BIOS */
extern void interrupt int10h(struct pregs r);	/* Video */
extern void interrupt int11h(struct pregs r);	/* Equipment word */
extern void interrupt int12h(struct pregs r);	/* Memory size */
extern void interrupt int13h(struct pregs r);	/* Disk */
extern void interrupt int14h(struct pregs r);	/* Serial */
extern void interrupt int15h(struct pregs r);	/* Cassette */
extern void interrupt int16h(struct pregs r);	/* Keyboard */
extern void interrupt int17h(struct pregs r);	/* Parallel */
extern void interrupt int18h(void);				/* ROM BASIC */
extern void interrupt int19h(void);				/* Boot from disk */
extern void interrupt int1Ah(struct pregs r);	/* Time */
extern void interrupt int1Bh(void);				/* User Ctrl-Break */
extern void interrupt int1Ch(void);				/* User timer tick */
extern void interrupt int4Ah(void);				/* User RTC alarm */

/* DDPTs */
extern struct ddpt ddpt250;		/* 8" 250KB */
extern struct ddpt ddpt360;		/* 5 1/4" 360KB */
extern struct ddpt ddpt1200;	/* 5 1/4" 1.2MB */
extern struct ddpt ddpt720;		/* 3 1/2" 720KB */
extern struct ddpt ddpt1440;	/* 3 1/2" 1.44MB */
extern struct ddpt ddpt2880;	/* 3 1/2" 2.88MB */
extern struct fdpt deffdpt;

/* Serial port */
extern void seroutb(char c);
extern char serinb(void);
extern char seroust(void);
extern char serinst(void);
extern unsigned char serlsr(void);
extern void serinit(unsigned char lcr, unsigned int dl);
extern void seroutstr(const char *str);
extern void serouthn(unsigned char c);
extern void serouthb(unsigned char c);
extern void serouthw(unsigned int i);
extern unsigned char serinhn(void);
extern unsigned char serinhb(void);
extern unsigned int serinhw(void);
extern void seroutd(int i);

/* PATB */
#define patb_size 2792
extern const unsigned char far patb[patb_size];

/* BIOS console I/O */
extern void conoutb(const char c);
extern char coninb(void);
extern void conoutstr(const char *s);
extern void conouthn(unsigned char c);
extern void conouthb(unsigned char c);
extern void conouthw(unsigned int i);
extern int bintobcd(int i);
extern int bcdtobin(int i);
extern void panic(const char *msg);

/* System initialisation */
extern void probe_com(void);
extern void probe_lpt(void);

#define sti() asm sti
#define cli() asm cli
#define nop() asm nop
#define hlt() asm hlt

extern void seroutb(char c);
extern char serinb(void);
extern char seroust(void);
extern char serinst(void);
extern void outstr(const char *s);

extern void serouthn(unsigned char c);
extern void serouthb(unsigned char c);
extern void serouthw(unsigned int i);
extern void serouthl(unsigned long l);
extern unsigned char serinhn(void);
extern unsigned char serinhb(void);
extern unsigned int serinhw(void);
extern unsigned long serinhl(void);

extern int bintobcd(int i);
extern int bcdtobin(int i);
extern void seroutd(int i);
extern char *serinl(char *buf, int bufsz);

extern void setintvec(int intno, void interrupt (*handler)());
extern void interrupt (* getintvec(int intno))();

extern void soundhz(int hz);

extern void pokesb(unsigned int pseg, unsigned int poff, unsigned char pdata);
extern unsigned char peeksb(unsigned int pseg, unsigned int poff);
extern void pokesw(unsigned int pseg, unsigned int poff, unsigned int pdata);
extern unsigned int peeksw(unsigned int pseg, unsigned int poff);

extern void panic(char *msg);
extern void far *segtofar(unsigned int pseg, unsigned int poff);
extern void pushcli(void);
extern void popcli(void);

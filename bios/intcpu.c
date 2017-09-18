#include "bios.h"

static void regdump(struct pregs *rp, char *msg)
{
	unsigned int rss;
	asm mov rss, ss;
	conoutstr("\r\n ** Unhandled interrupt: ");
	conoutstr(msg);
	conoutstr(", registers follow: **\r\n");

	conoutstr("CS=");
	conouthw(rp->cs);
	conoutstr(", DS=");
	conouthw(rp->ds);
	conoutstr(", ES=");
	conouthw(rp->es);
	conoutstr(", SS=");
	conouthw(rss);
	conoutstr(", IP=");
	conouthw(rp->ip);
	conoutstr(", BP=");
	conouthw(rp->bp);
	conoutstr(", flags=");
	conouthw(rp->flags);
	conoutstr("\r\nAX=");
	conouthw(rp->ax);
	conoutstr(", BX=");
	conouthw(rp->bx);
	conoutstr(", CX=");
	conouthw(rp->cx);
	conoutstr(", DX=");
	conouthw(rp->dx);
	conoutstr(", SI=");
	conouthw(rp->si);
	conoutstr(", DI=");
	conouthw(rp->di);
	panic("System halted");
}

void interrupt intnul(void) {}
void interrupt intunh(struct pregs r) { regdump(&r, ">= 20h"); }

void interrupt int00h(struct pregs r) { regdump(&r, "Divide error"); }
void interrupt int01h(struct pregs r) { regdump(&r, "Single step"); }
void interrupt int02h(struct pregs r) { regdump(&r, "NMI"); }
void interrupt int03h(struct pregs r) { regdump(&r, "Breakpoint"); }
void interrupt int04h(struct pregs r) { regdump(&r, "INTO overflow"); }
void interrupt int05h(struct pregs r) { regdump(&r, "Array bounds"); }
void interrupt int06h(struct pregs r) { regdump(&r, "Invalid opcode"); }
void interrupt int07h(struct pregs r) { regdump(&r, "ESC opcode"); }

void interrupt int09h(struct pregs r) { regdump(&r, "09h (reserved)"); }
void interrupt int0Ah(struct pregs r) { regdump(&r, "DMA 0"); }
void interrupt int0Bh(struct pregs r) { regdump(&r, "DMA 1"); }
void interrupt int0Eh(struct pregs r) { regdump(&r, "INT2 (PPI)"); }
void interrupt int0Fh(struct pregs r) { regdump(&r, "INT3 (expansion)"); }

/* Interrupt 11h
 * Get equipment list
 */
void interrupt int11h(struct pregs r)
{
	// Return equipment word from the BIOS data area
	r.ax = *((volatile unsigned int far *) (BDA+0x10));
}

/* Interrupt 12h
 * Get memory size
 */
void interrupt int12h(struct pregs r)
{
	// Return contiguous memory size in KB from the BIOS data area
	r.ax = *((volatile unsigned int far *) (BDA+0x13));
}

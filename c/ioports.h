#define F_CPU		10000000UL		/* CPU frequency is 1/2 the xtal */
#define F_UART		2457600UL		/* UART baud rate xtal */
#define uartdll(baud) ((F_UART/(16*baud)) & 0xFF)
#define uartdlm(baud) (((F_UART/(16*baud)) >> 8) & 0xFF)

/* External peripheral interface */
#define EP_BASE		0xF800
#define EP_PCS0		EP_BASE
#define EP_PCS1		EP_BASE+0x80
#define EP_PCS2		EP_BASE+0x100
#define EP_PCS3		EP_BASE+0x180
#define EP_PCS4		EP_BASE+0x200
#define EP_PCS5		EP_BASE+0x280
#define EP_PCS6		EP_BASE+0x300

/* DS17887 RTC */
#define RTC_BASE	EP_PCS0
#define RTC_SEC		RTC_BASE		/* Seconds */
#define RTC_SECAL	RTC_BASE+0x01	/* Seconds alarm */
#define RTC_MIN		RTC_BASE+0x02	/* Minutes */
#define RTC_MINAL	RTC_BASE+0x03	/* Minutes alarm */
#define RTC_HR		RTC_BASE+0x04	/* Hours */
#define RTC_HRAL	RTC_BASE+0x05	/* Hours alarm */
#define RTC_DAY		RTC_BASE+0x06	/* Day of week (01-07) */
#define RTC_DATE	RTC_BASE+0x07	/* Date (01-31) */
#define RTC_MTH		RTC_BASE+0x08	/* Month (01-12) */
#define RTC_YR		RTC_BASE+0x09	/* Year (00-99) */
#define RTC_CTLA	RTC_BASE+0x0A	/* Control A (Osc enable, bank select, SQW rate) */
#define RTC_CTLB	RTC_BASE+0x0B	/* Control B (Interrupt enables etc) */
#define RTC_CTLC	RTC_BASE+0x0C	/* Control C (Interrupt flags) */
#define RTC_CTLD	RTC_BASE+0x0D	/* Control D (Battery monitor) */
#define RTC_RAM		RTC_BASE+0x0E	/* User RAM (50 bytes common to both banks) */

#define RB0_RAM		RTC_BASE+0x40	/* User RAM (additional 64 bytes) */

#define RB1_MODEL	RTC_BASE+0x40	/* Serial number (model) */
#define RB1_SER1	RTC_BASE+0x41	/* Serial number (1st byte) */
#define RB1_SER2	RTC_BASE+0x42	/* Serial number (2nd byte) */
#define RB1_SER3	RTC_BASE+0x43	/* Serial number (3rd byte) */
#define RB1_SER4	RTC_BASE+0x44	/* Serial number (4th byte) */
#define RB1_SER5	RTC_BASE+0x45	/* Serial number (5th byte) */
#define RB1_SER6	RTC_BASE+0x46	/* Serial number (6th byte) */
#define RB1_CRC		RTC_BASE+0x47	/* Serial number (CRC) */
#define RB1_CENT	RTC_BASE+0x48	/* Century */
#define RB1_DATAL	RTC_BASE+0x49	/* Date alarm */
#define RB1_EC4A	RTC_BASE+0x4A	/* Extended control 4A */
#define RB1_EC4B	RTC_BASE+0x4B	/* Extended control 4B */
#define RB1_SMI2	RTC_BASE+0x4E	/* SMI recovery stack (RTC address - 2) */
#define RB1_SMI3	RTC_BASE+0x4F	/* SMI recovery stack (RTC address - 3) */
#define RB1_ERAL	RTC_BASE+0x50	/* Extended RAM address (LSB) */
#define RB1_ERAH	RTC_BASE+0x51	/* Extended RAM address (MSB) */
#define RB1_ERD		RTC_BASE+0x53	/* Extended RAM data */
#define RB1_WCTR	RTC_BASE+0x5E	/* RTC write counter */

/* 8250 UART */
#define UART_BASE	EP_PCS1
#define UART_RBR	UART_BASE		/* Receiver buffer register (read) */
#define UART_THR	UART_BASE		/* Transmitter holding register (write) */
#define UART_IER	UART_BASE+0x1	/* Interrupt enable register */
#define UART_IIR	UART_BASE+0x2	/* Interrupt identification register (read) */
#define UART_LCR	UART_BASE+0x3	/* Line control register */
#define UART_MCR	UART_BASE+0x4	/* Modem control register */
#define UART_LSR	UART_BASE+0x5	/* Line status register */
#define UART_MSR	UART_BASE+0x6	/* Modem status register */
#define UART_SCR	UART_BASE+0x7	/* Scratch register */

#define UART_DLL	UART_BASE		/* Divisor latch (LSB) (when DLAB=1) */
#define UART_DLM	UART_BASE+0x1	/* Divisor latch (MSB) (when DLAB=1) */

/* 8255 PPI */
#define PPI_BASE	EP_PCS2
#define PPI_PA		PPI_BASE		/* Port A */
#define PPI_PB		PPI_BASE+0x1	/* Port B */
#define PPI_PC		PPI_BASE+0x2	/* Port C */
#define PPI_CTL		PPI_BASE+0x3	/* Control word */

/* Internal peripheral interface */
#define IP_BASE		0xFF00			/* Control block base address */
#define IP_RR		IP_BASE+0xFE	/* Relocation register */
#define IP_PDCON	IP_BASE+0xF0	/* Power-save control register (enhanced mode only) */

/* DRAM refresh (enhanced mode only) */
#define IDR_MDRAM	IP_BASE+0xE0	/* Memory partition register */
#define IDR_CDRAM	IP_BASE+0xE2	/* Clock pre-scaler register */
#define IDR_EDRAM	IP_BASE+0xE4	/* Enable RCU register */

/* Chip selects */
#define ICS_UMCS	IP_BASE+0xA0	/* Upper memory base address */
#define ICS_LMCS	IP_BASE+0xA2	/* Lower memory end address */
#define ICS_PACS	IP_BASE+0xA4	/* Peripheral base address */
#define ICS_MMCS	IP_BASE+0xA6	/* Middle memory base address */
#define ICS_MPCS	IP_BASE+0xA8	/* Middle memory size, peripheral mapping */

/* DMA */
#define ID0_CW		IP_BASE+0xCA	/* Control word */
#define ID0_TC		IP_BASE+0xC8	/* Transfer count */
#define ID0_DPH		IP_BASE+0xC6	/* Destination pointer, upper 4 bits */
#define ID0_DP		IP_BASE+0xC4	/* Destination pointer */
#define ID0_SPH		IP_BASE+0xC2	/* Source pointer, upper 4 bits */
#define ID0_SP		IP_BASE+0xC0	/* Source pointer */

#define ID1_CW		IP_BASE+0xDA	/* Control word */
#define ID1_TC		IP_BASE+0xD8	/* Transfer count */
#define ID1_DPH		IP_BASE+0xD6	/* Destination pointer, upper 4 bits */
#define ID1_DP		IP_BASE+0xD4	/* Destination pointer */
#define ID1_SPH		IP_BASE+0xD2	/* Source pointer, upper 4 bits */
#define ID1_SP		IP_BASE+0xD0	/* Source pointer */

/* Timers */
#define IT0_CW		IP_BASE+0x56	/* Control word */
#define IT0_MCB		IP_BASE+0x54	/* Max count B */
#define IT0_MCA		IP_BASE+0x52	/* Max count A */
#define IT0_CR		IP_BASE+0x50	/* Count register */

#define IT1_CW		IP_BASE+0x5E	/* Control word */
#define IT1_MCB		IP_BASE+0x5C	/* Max count B */
#define IT1_MCA		IP_BASE+0x5A	/* Max count A */
#define IT1_CR		IP_BASE+0x58	/* Count register */

#define IT2_CW		IP_BASE+0x66	/* Control word */
#define IT2_MCA		IP_BASE+0x62	/* Max count A */
#define IT2_CR		IP_BASE+0x60	/* Count register */

/* Interrupt controller */
#define IIM_INT3	IP_BASE+0x3E	/* INT3 control */
#define IIM_INT2	IP_BASE+0x3C	/* INT2 control */
#define IIM_INT1	IP_BASE+0x3A	/* INT1 control */
#define IIM_INT0	IP_BASE+0x38	/* INT0 control */
#define IIM_DMA1	IP_BASE+0x36	/* DMA 1 control */
#define IIM_DMA0	IP_BASE+0x34	/* DMA 0 control */
#define IIM_TIM		IP_BASE+0x32	/* Timer control */
#define IIM_IST		IP_BASE+0x30	/* Interrupt status */
#define IIM_IRQ		IP_BASE+0x2E	/* Interrupt request */
#define IIM_IS		IP_BASE+0x2C	/* In-service */
#define IIM_PLM		IP_BASE+0x2A	/* Priority level mask */
#define IIM_MSK		IP_BASE+0x28	/* Mask */
#define IIM_PLST	IP_BASE+0x26	/* Poll status */
#define IIM_POLL	IP_BASE+0x24	/* Poll */
#define IIM_EOI		IP_BASE+0x22	/* End-of-interrupt */

#define IIS_TIM2	IP_BASE+0x3A	/* Timer 2 control */
#define IIS_TIM1	IP_BASE+0x38	/* Timer 1 control */
#define IIS_DMA1	IP_BASE+0x36	/* DMA 1 control */
#define IIS_DMA0	IP_BASE+0x34	/* DMA 0 control */
#define IIS_TIM0	IP_BASE+0x32	/* Timer 0 control */
#define IIS_IST		IP_BASE+0x30	/* Interrupt status */
#define IIS_IRQ		IP_BASE+0x2E	/* Interrupt request */
#define IIS_IS		IP_BASE+0x2C	/* In-service */
#define IIS_PLM		IP_BASE+0x2A	/* Priority level mask */
#define IIS_MSK		IP_BASE+0x28	/* Mask */
#define IIS_EOI		IP_BASE+0x22	/* End-of-interrupt */
#define IIS_IV		IP_BASE+0x20	/* Interrupt vector */

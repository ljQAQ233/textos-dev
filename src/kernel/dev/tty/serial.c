/*
 * x86 platform serial port (UART) implementation
 */
#include <io.h>
#include <irq.h>
#include <intr.h>
#include <textos/errno.h>
#include <textos/dev/tty/tty.h>
#include <textos/klib/vsprintf.h>

//      name base  irq number
#define COM1 0x3f8 // 4
#define COM2 0x2f8 // 3
#define COM3 0x3e8 // 4
#define COM4 0x2e8 // 3

//      Name    Offset  DLAB
#define R_DATA  0     // 0 Receive / Transfer
#define R_INTR  1     // 0 Interrupt Enable Register
#define R_LSB   0     // 1 Divisor Baud
#define R_MSB   1     // 1 
#define R_FIFO  2     // - Interrupt Identification / FIFO Control Register
#define R_LCR   3     // - The most significant bit of this register is the DLAB.
#define R_MCR   4     // - Modem Control Register
#define R_LSR   5     // - Line Status Register
#define R_MSR   6     // - Modem Status Register
#define R_SCR   7     // - Scratch Register

#define LCR_DLAB (1 << 7)

#define LSR_DR (1 << 0)   // Set if there is data that can be read
#define LSR_OE (1 << 1)   // Set if there has been data lost
#define LSR_PE (1 << 2)   // Set if there was an error in the transmission as detected by parity
#define LSR_FE (1 << 3)   // Set if a stop bit was missing
#define LSR_BI (1 << 4)   // Set if there is a break in data input
#define LSR_THRE (1 << 5) // Set if the transmission buffer is empty (i.e. data can be sent)
#define LSR_TEMT (1 << 6) // Set if the transmitter is not doing anything
#define LSR_IMPE (1 << 7) // Set if there is an error with a word in the input buffer

#define ERBFI (1 << 0) // interrupt if Received Data Available
#define ETBEI (1 << 1) // interrupt if Transmitter Holding Register Empty

typedef struct
{
    u16 base;
    u16 irqnr;
    tty_t *tty;
} serial_t;

static serial_t srls[] = {
    { COM1, 4, 0 },
    { COM2, 3, 0 },
    { COM3, 4, 0 },
    { COM4, 3, 0 },
};

static inline char sgetc(serial_t *io)
{
    return inb(io->base + R_DATA);
}

static inline void sputc(serial_t *io, char c)
{
    while ((inb(io->base + R_LSR) & LSR_THRE) == 0);

    outb(io->base + R_DATA, c);
}

int serial_ioctl(serial_t *io, int req, void *argp)
{
    return -EINVAL;
}

ssize_t serial_write(serial_t *io, char *s, size_t len)
{
    char *p = s;
    UNINTR_AREA({
        for ( ; len ; p++, len--)
            sputc(io, *p++);
    });
    return (ssize_t)(p - s);
}

extern void __tty_rxb(tty_t *tty, char *buf, size_t len);

__INTR_HANDLER(serial_handler)
{
    for (int i = 0 ; i < 4 ; i++)
    {
        serial_t *srl = &srls[i];
        u8 lsr = inb(srl->base + R_LSR);
        if (~lsr & LSR_DR)
            continue;
        
        char c = sgetc(srl);
        if (srl->tty)
            __tty_rxb(srl->tty, &c, 1);
    }
    lapic_sendeoi();
}

static void init(int idx)
{
    serial_t *srl = &srls[idx];
    outb(srl->base + R_LCR, 0);            // Disable DLAB
    outb(srl->base + R_INTR, ERBFI|ETBEI); // Enable interrupts
    outb(srl->base + R_LCR, LCR_DLAB);     // Enable DLAB
    outb(srl->base + R_LSB, 1);            // Set baud : 115200
    outb(srl->base + R_MSB, 0);            // High
    outb(srl->base + R_LCR, 0b11);         // 7 bits for data and 1 stop bit
    outb(srl->base + R_FIFO, 0b00000001);  // Enable FIFO and let trigger level 14

    u8 mcr0 = inb(srl->base + R_MCR);
    outb(srl->base + R_MCR, 0b11110);      // start to check
    outb(srl->base + R_DATA, 0xae);
    if (inb(srl->base + R_DATA) != 0xae)
    {
        return;
    }
    outb(srl->base + R_MCR, 0b100);

    char ttyname[16];
    sprintf(ttyname, "ttyS%d", idx);
    srl->tty = tty_register(
        NULL, ttyname, srl,
        (void *)serial_ioctl,
        (void *)NULL,
        (void *)serial_write);
}

#include <lai/helpers/resource.h>

void serial_init()
{
    init(0);
    init(1);
    init(2);
    init(3);

    ioapic_rteset(3, _IOAPIC_RTE(INT_SERIAL));
    ioapic_rteset(4, _IOAPIC_RTE(INT_SERIAL));
    intr_register(INT_SERIAL, serial_handler);
}


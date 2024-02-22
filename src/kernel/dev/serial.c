#include <textos/dev.h>

#include <io.h>

#define COM1 0x3f8

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

static inline char serial_getc ()
{
    while ((inb (COM1 + R_LSR) & 0x01) == 0);

    return inb (COM1 + R_DATA);
}

size_t serial_read (dev_t *dev, char *str, size_t siz)
{
    for (size_t i = 0 ; i < siz ; i++)
        *str++ = serial_getc (); 

    return siz;
}

static inline void serial_putc (char Char)
{
    while ((inb (COM1 + R_LSR) & 0x20) == 0);

    outb (COM1 + R_DATA, Char);
}

size_t serial_write (dev_t *dev, char *str, size_t siz)
{
    char *p = str;
    while (siz-- && p && *p)
        serial_putc (*p++);

    return (size_t)(p - str);
}

static dev_pri_t serial = {
    .dev = &(dev_t) {
        .name = "serial port",
        .read = (void *)serial_read,
        .write = (void *)serial_write,
        .type = DEV_CHAR,
        .subtype = DEV_SERIAL,
    }
};

void serial_init ()
{
    outb (COM1 + R_INTR , 0);          // Disable interrupts
    outb (COM1 + R_LCR  , 1 << 7);     // Enable DLAB
    outb (COM1 + R_LSB  , 1);          // Set baud : 115200
    outb (COM1 + R_MSB  , 0);          // High
    outb (COM1 + R_LCR  , 0b11);       // 7 bits for data and 1 stop bit
    outb (COM1 + R_FIFO , 0b11000001); // Enable FIFO and let trigger level 14

    outb (COM1 + R_MCR  , 0b11110);    // 开启环回,检测开始

    outb (COM1 + R_DATA , 0xae);
    if (inb (COM1 + R_DATA) != 0xae)
    {
        return;
    }

    outb (COM1 + R_MCR, 0b100);        // 恢复 -> Out 1
    
    __dev_register (&serial);
}


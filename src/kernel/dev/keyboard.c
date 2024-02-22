#define R_DATA 0x60
#define R_STAT 0x64
#define R_CMD  0x64

// #define S_OUT (1)
// #define S_IN  (1 << 1)
// #define S_SYS (1 << 2)

#define CMD_PORT1_OFF 0xAD
#define CMD_PORT1_ON  0xAE
#define CMD_PORT2_OFF 0xA7
#define CMD_PORT2_ON  0xA8

#define CMD_PORT1_TEST 0xAB
#define CMD_PORT2_TEST 0xA9

#define CMD_W_CTL  0x60 // Controller Configuration Byte
#define CMD_R_CTL  0x20

#define CMD_W_CTLOUT 0xD1 // PS/2 Controller Output Port
#define CMD_R_CTLOUT 0xD0

/* Controller Configuration Byte */
#define CTL_INT1_ON (1)
#define CTL_INT2_ON (1 << 1)
#define CTL_SYS_PS  (1 << 2)
#define CTL_CLOCK1  (1 << 4)
#define CTL_CLOCK2  (1 << 5)
#define CTL_TRSAN1  (1 << 6) // First PS/2 port transiation

#define S_IN_FULL  (1)
#define S_OUT_FULL (1 << 1)
#define S_SYS      (1 << 2)
#define S_CTL_CMD  (1 << 3)
#define S_TIMEOUT  (1 << 6)
#define S_PERROR   (1 << 7)

#include <io.h>
#include <textos/debug.h>

#include <irq.h>
#include <intr.h>

static void input_clear()
{
    while (inb(R_STAT) & S_IN_FULL) inb(R_DATA);
}

__INTR_HANDLER(keyboard_handler)
{
    lapic_sendeoi();

    u8 chr = 0;
    while (!(inb(R_STAT) & S_IN_FULL));
    chr = inb(R_DATA);

    DEBUGK(K_KBD, "pressed : %u\n", chr);
}

#include <textos/dev.h>
#include <textos/panic.h>

void keyboard_init()
{
    // 禁用所有设备
    outb(R_CMD, CMD_PORT1_OFF);
    outb(R_CMD, CMD_PORT2_OFF);

    input_clear();

    // 自检,如果没有通过则不能进入系统.
    outb(R_CMD, CMD_PORT1_TEST);
    if (!(inb(R_STAT) & S_SYS))
        PANIC("keyboard cannot pass self test!\n");
    
    input_clear();
    
    intr_register(INT_KEYBOARD, keyboard_handler);
    ioapic_rteset(IRQ_KEYBOARD, _IOAPIC_RTE(INT_KEYBOARD));

    // 打开端口与中断
    outb(R_CMD, CMD_PORT1_ON);
    outb(R_CMD, CMD_W_CTL);
    outb(R_DATA, CTL_INT1_ON);

    dev_t *dev = dev_new();
    dev->name = "ps/2 keyboard";
    dev->read = NULL;
    dev->write = NULL;
    dev->type = DEV_CHAR;
    dev->subtype = DEV_KBD;
    dev_register (dev);
    DEBUGK(K_INIT, "kbd initialized!\n");
}


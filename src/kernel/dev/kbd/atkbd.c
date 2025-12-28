#include <io.h>
#include <irq.h>
#include <intr.h>
#include <textos/task.h>
#include <textos/dev/keys.h>
#include <textos/dev/kbd/sc1.h>

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
#define CTL_TRSAN1  (1 << 6) // First PS/2 port transiation (to scancode 1)

/* Status (R_STAT) */
#define S_IN_FULL  (1)
#define S_OUT_FULL (1 << 1)
#define S_SYS      (1 << 2)
#define S_CTL_CMD  (1 << 3)
#define S_TIMEOUT  (1 << 6)
#define S_PERROR   (1 << 7)

static keysym_t flag = 0;
extern void __tty_rx(void *tty, keysym_t sym);

/*
 * 注意中断发生的时机:
 *  - ctrl / alt / shift / capslock / super / app 按下与松开发出中断
 *  - 其他按键产生连续中断
 * number lock 与 caps lock 是通一次变一次的
 */
__INTR_HANDLER(keyboard_handler)
{
    lapic_sendeoi();
    while (!(inb(R_STAT) & S_IN_FULL));

    u8 code = inb(R_DATA);
    bool brk = code & 0x80;
    keysym_t sym = kbd_sc1_to_sym(code);
    if (sym & KEY_S_WAIT)
        return;
    if (sym & KEY_S_ERROR)
        return;
    if (sym & KEY_S_MASK)
    {
        if (brk)
        {
            sym &= ~KEY_S_CAPSLK;
            sym &= ~KEY_S_NUMLK;
        }
        if (brk)
            flag &= ~sym;
        else
            flag |= sym;
        return;
    }
    if (brk)
        return;
    __tty_rx(NULL, sym | flag);
}

void kayboard_light(bool x)
{
    // TODO
}

static void input_clear()
{
    while (inb(R_STAT) & S_IN_FULL) inb(R_DATA);
}

#include <textos/dev.h>
#include <textos/panic.h>

/*
 * The period of configuration mainly includes 2 parts:
 *   - Cmd
 *   - Cfg byte
 * 1. Cmd -> Write cmds to port `R_CMD`
 * 2. Cfg byte -> Write CMD_W_CTL to `R_CMD` and configuration byte to `R_DATA`
 */

void keyboard_init()
{
    // 禁用所有设备
    outb(R_CMD, CMD_PORT1_OFF);
    outb(R_CMD, CMD_PORT2_OFF);

    input_clear();
    intr_register(INT_KEYBOARD, keyboard_handler);
    ioapic_rteset(IRQ_KEYBOARD, _IOAPIC_RTE(INT_KEYBOARD));

    // 打开端口与中断
    outb(R_CMD, CMD_PORT1_ON);
    outb(R_CMD, CMD_W_CTL);
    outb(R_DATA, CTL_INT1_ON | CTL_TRSAN1);

    devst_t *dev = dev_new();
    dev->name = "atkbd";
    dev->read = NULL;
    dev->write = NULL;
    dev->type = DEV_CHAR;
    dev->subtype = DEV_KBD;
    DEBUGK(K_INFO, "atkbd initialized!\n");
}


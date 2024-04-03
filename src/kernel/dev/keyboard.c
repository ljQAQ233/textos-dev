#include <textos/dev.h>
#include <textos/mm.h>

#define INPUT_MAX 0x100

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

#include <io.h>
#include <textos/debug.h>
#include <textos/klib/ring.h>

static ring_t ikey;
static char   ibuf[INPUT_MAX];

static bool shift = false;
static bool alt   = false;
static bool ctrl  = false;
static bool caps  = false;

static void _ring_init()
{
    ring_init (&ikey, ibuf, sizeof(ibuf), sizeof(char));
}

static char keycode[][2] = {
    { '\0'    , '\0'},  // (nul)
    { '^'     , '^' },  // ESC
    { '1'     , '!' },
    { '2'     , '@' },
    { '3'     , '#' },
    { '4'     , '$' },
    { '5'     , '%' },
    { '6'     , '^' },
    { '7'     , '&' },
    { '8'     , '*' },
    { '9'     , '(' },
    { '0'     , ')' },
    { '-'     , '_' },
    { '='     , '+' },
    { '\0'    , '\0'}, // BACKSPACE
    { '\t'    , '\t'},
    { 'q'     , 'Q' },
    { 'w'     , 'W' },
    { 'e'     , 'E' },
    { 'r'     , 'R' },
    { 't'     , 'T' },
    { 'y'     , 'Y' },
    { 'u'     , 'U' },
    { 'i'     , 'I' },
    { 'o'     , 'O' },
    { 'p'     , 'P' },
    { '['     , '{' },
    { ']'     , '}' },
    { '\n'    , '\n'},
    { '\0'    , '\0'},
    { 'a'     , 'A' },
    { 's'     , 'S' },
    { 'd'     , 'D' },
    { 'f'     , 'F' },
    { 'g'     , 'G' },
    { 'h'     , 'H' },
    { 'j'     , 'J' },
    { 'k'     , 'K' },
    { 'l'     , 'L' },
    { ';'     , ':' },
    { '\''    , '"' },
    { '`'     , '~' },
    { '\0'    , '\0'}, // LEFTSHIFT
    { '\\'    , '|' },
    { 'z'     , 'Z' },
    { 'x'     , 'X' },
    { 'c'     , 'C' },
    { 'v'     , 'V' },
    { 'b'     , 'B' },
    { 'n'     , 'N' },
    { 'm'     , 'M' },
    { ','     , '<' },
    { '.'     , '>' },
    { '/'     , '?' },
    { '\0'    , '\0'}, // RIGHTSHIFT
    { '*'     , '*' }, // TODO : Check it
    { '\0'    , '\0'}, // LEFTALT
    { ' '     , ' ' }, // SPACE
    { '\0'    , '\0'}, // CAPSLOCK
    { '\0'    , '\0'}, // F1
    { '\0'    , '\0'}, // F2
    { '\0'    , '\0'}, // F3
    { '\0'    , '\0'}, // F4
    { '\0'    , '\0'}, // F5
    { '\0'    , '\0'}, // F6
    { '\0'    , '\0'}, // F7
    { '\0'    , '\0'}, // F8
    { '\0'    , '\0'}, // F9
    { '\0'    , '\0'}, // F10
    { '\0'    , '\0'}, // NUMLOCK
    { '\0'    , '\0'}, // SCRLOCK
    { '7'     , '7' }, // KBD7
    { '8'     , '8' }, // KBD8
    { '9'     , '9' }, // KBD9
    { '-'     , '-' }, // KBD-
    { '4'     , '4' }, // KBD4
    { '5'     , '5' }, // KBD5
    { '6'     , '6' }, // KBD6
    { '+'     , '+' }, // KBD+
    { '1'     , '1' }, // KBD1
    { '2'     , '2' }, // KBD2
    { '3'     , '3' }, // KBD3
    { '0'     , '0' }, // KBD0
    { '.'     , '.' }, // KBD.
    { '\0'    , '\0'}, // (nul)
    { '\0'    , '\0'}, // (nul)
    { '\0'    , '\0'}, // (nul)
    { '\0'    , '\0'}, // F11
    { '\0'    , '\0'}, // F12
};


#include <irq.h>
#include <intr.h>
#include <textos/task.h>

/* The temporary solution */
static int pid = -1;

__INTR_HANDLER(keyboard_handler)
{
    lapic_sendeoi();

    while (!(inb(R_STAT) & S_IN_FULL));
    u8 code = inb(R_DATA);

    bool brk = code & 0x80;
    code &= code &~ 0x80;

    /*
       Shift , Ctrl 按键被按下后只会产生两次中断,即
        
        - Press
        - Release
    */
    switch (code) {
        case 0x2a: // left
        case 0x36: // right
            shift = !shift;
            return;
        case 0x1d: // left
            ctrl = !ctrl;
            return;
        case 0x3a:
            if (brk) caps = !caps;
            return;
        case 0x38: // left
            alt = brk;
            return;
        // case 0xe0:
        //     if (*((char *)RingGet(&Ring, Ring.Head)) == 0xb8) {
        //         return;
        //     }
    }

    if (brk)
        return;

    /* 如果开了 Shift , 则选取上端字符,
       如果此时 Caps 也开了 且 扫描码表示的是一个字母, 则将 Line 取逻辑非 */
    bool at = shift ? 1 : 0;
    if (caps && 'a' <= keycode[code][0] && keycode[code][0] <= 'z')
        at = !at;
    char chr = keycode[code][at];

    ring_push (&ikey, &chr);

    DEBUGK(K_KBD, "common key - %d%d%d - %c(%d)!\n", shift, ctrl, caps, chr, code);

    if (pid < 0)
        return;
    task_unblock (pid);
}

static char keyboard_getc()
{
    pid = task_current()->pid;
    while (ring_empty (&ikey))
        task_block();

    return *((char *)ring_pop(&ikey));
}

void keyboard_read(dev_t *dev, char *buf, size_t cnt)
{
    for (int i = 0 ; i < cnt ; i++) {
        char c = keyboard_getc();
        buf[i] = c;
    }
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
    The period of configuration mainly includes 2 parts:
      - Cmd
      - Cfg byte

    1. Cmd -> Write cmds to port `R_CMD`
    2. Cfg byte -> Write CMD_W_CTL to `R_CMD` and configuration byte to `R_DATA`
*/

void keyboard_init ()
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
    
    // 初始化 环形缓冲区
    _ring_init();
    intr_register(INT_KEYBOARD, keyboard_handler);
    ioapic_rteset(IRQ_KEYBOARD, _IOAPIC_RTE(INT_KEYBOARD));

    // 打开端口与中断
    outb(R_CMD, CMD_PORT1_ON);
    outb(R_CMD, CMD_W_CTL);
    outb(R_DATA, CTL_INT1_ON | CTL_TRSAN1);

    dev_t *dev = dev_new();
    dev->name = "ps/2 keyboard";
    dev->read = (void *)keyboard_read;
    dev->write = NULL;
    dev->type = DEV_CHAR;
    dev->subtype = DEV_KBD;
    dev_register (dev);
    DEBUGK(K_INIT, "kbd initialized!\n");
}


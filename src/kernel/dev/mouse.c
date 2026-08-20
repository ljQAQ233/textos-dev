#include <intr.h>
#include <io.h>
#include <irq.h>
#include <textos/dev/event.h>
#include <textos/dev/keys.h>

#define MAX_RESEND 3

#define sign(bit) ((bit) ? -1 : 1)

// mode: if 1, the current mode is remote mode; if 0 then it is stream mode
// enable: if 1, then data reporting is enabled; 0 means disabled
// scaling: if 1, scaling 2:1 is enabled; if 0 then scaling 1:1 is enabled.
struct status
{
    uint8_t right : 1;
    uint8_t middle : 1;
    uint8_t left : 1;
    uint8_t _zero0 : 1;
    uint8_t scaling : 1;
    uint8_t enable : 1;
    uint8_t mode : 1;
    uint8_t _zero1 : 1;
} _packed;

struct byte1
{
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t middle : 1;
    uint8_t _one0 : 1;
    uint8_t x_sign : 1;
    uint8_t y_sign : 1;
    uint8_t x_overflow : 1;
    uint8_t y_overflow : 1;
} _packed;

union packet
{
    struct
    {
        struct byte1 flags;
        uint8_t x_movement;
        uint8_t y_movement;
    } _packed;
    uint8_t raw[4];
} _packed;

static keysym_t status;
static int packet_size = 3;
static struct event_registry *evreg;

static void mouse_wait_input()
{
    // 等待 输入缓冲器空, 可以写
    while (inb(0x64) & 0x02)
        ;
}

static void mouse_wait_output()
{
    // 等待 输出缓冲器不为空, 可以读
    while (~inb(0x64) & 0x01)
        ;
}

static void mouse_wait_ack()
{
    mouse_wait_output();
    uint8_t ack = inb(0x60);
    DEBUGK(K_TRACE, "mouse ack = %#x\n", (int)ack);
}

#define before_out() mouse_wait_input()
#define before_in()  mouse_wait_output()

static void mouse_clear_output()
{
    while (inb(0x64) & 0x01) {
        uint8_t in = inb(0x60);
        DEBUGK(K_TRACE, "clear input = %#x\n", (int)in);
    }
}

static int mouse_enable_irq()
{
    mouse_clear_output();

    // 读取 compaq status byte
    before_out();
    outb(0x64, 0x20);

    before_in();
    uint8_t status = inb(0x60);
    status |= (1 << 1);
    status &= ~(1 << 5);

    // 写入
    before_out();
    outb(0x64, 0x60);
    before_out();
    outb(0x60, status);

    // 可能会有 ACK 产生
    return 0;
}

static int mouse_enable_aux()
{
    mouse_clear_output();

    // 0xa8 Aux Input Enable Command
    before_out();
    outb(0x64, 0xa8);

    return 0;
}

static int mouse_enable_packet()
{
    mouse_clear_output();

    before_out();
    outb(0x64, 0xd4);
    before_out();
    outb(0x60, 0xf4);

    return 0;
}

static int mouse_get_id()
{
    mouse_clear_output();

    // 0xf2 读取 mouse id
    before_out();
    outb(0x64, 0xd4);
    before_out();
    outb(0x60, 0xf2);

    mouse_wait_ack();
    return inb(0x60);
}

static int mouse_set_sampling(uint8_t rate)
{}

static int mouse_init_wheel()
{
    mouse_set_sampling(200);
    mouse_set_sampling(100);
    mouse_set_sampling(80);
    return mouse_get_id() == 3;
}

static int mouse_init_extra()
{
    mouse_set_sampling(200);
    mouse_set_sampling(200);
    mouse_set_sampling(80);
    return mouse_get_id() == 4;
}

static int mouse_rx_packet(union packet *pkt)
{
    for (int i = 0; i < packet_size; i++) {
        if (~inb(0x64) & 0x20) return -1;
        pkt->raw[i] = inb(0x60);
    }
    return packet_size;
}

__INTR_HANDLER(mouse_handler)
{
    lapic_sendeoi();
    int pktsz = -1;
    union packet pkt;

    for (int i = 0; i < MAX_RESEND && pktsz < 0; i++)
        pktsz = mouse_rx_packet(&pkt);
    if (pktsz < 0) return;
    if (pkt.flags.x_overflow || pkt.flags.y_overflow) return;

    keysym_t k = 0;
    if (pkt.flags.left) k |= KEY_S_MOUSE_LEFT;
    if (pkt.flags.right) k |= KEY_S_MOUSE_RIGHT;
    if (pkt.flags.middle) k |= KEY_S_MOUSE_MIDDLE;
    status = k;

    int dx = sign(pkt.flags.x_sign) * pkt.x_movement;
    int dy = sign(pkt.flags.y_sign) * pkt.y_movement;
    DEBUGK(K_TRACE, "mouse input: dx=%d dy=%d status=%d\n", dx, dy, status);

    event_push_mouse(evreg, status, dx, dy);
}

void mouse_init()
{
    int id;
    mouse_enable_irq();
    mouse_enable_aux();
    mouse_enable_packet();

    id = mouse_get_id();
    DEBUGK(K_INFO, "mouse id = %d\n", id);

    // mouse_init_wheel();
    // mouse_init_extra();

    ioapic_rteset(IRQ_MOUSE, INT_MOUSE);
    intr_register(INT_MOUSE, mouse_handler);

    mouse_clear_output();

    evreg = event_register(EV_MOUSE);
}

/*
 * e1000 (emu) driver
*/

#include <io.h>
#include <irq.h>
#include <intr.h>
#include <string.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>
#include <textos/dev/pci.h>
#include <textos/panic.h>
#include <textos/assert.h>

#define INTEL_VEND    0x8086
#define E1000_EMU     0x100E
#define E1000E_EMU    0x10d3

#define RCTL_EN            (1 << 1)  // Receiver Enable
#define RCTL_SBP           (1 << 2)  // Store Bad Packets
#define RCTL_UPE           (1 << 3)  // Unicast Promiscuous Enabled
#define RCTL_MPE           (1 << 4)  // Multicast Promiscuous Enabled
#define RCTL_LPE           (1 << 5)  // Long Packet Reception Enable
#define RCTL_LBM_NONE      (0 << 6)  // No Loopback
#define RCTL_LBM_PHY       (3 << 6)  // PHY or external SerDesc loopback
#define RTCL_RDMTS_HALF    (0 << 8)  // Free Buffer Threshold is 1/2 of RDLEN
#define RTCL_RDMTS_QUARTER (1 << 8)  // Free Buffer Threshold is 1/4 of RDLEN
#define RTCL_RDMTS_EIGHTH  (2 << 8)  // Free Buffer Threshold is 1/8 of RDLEN
#define RCTL_MO_36         (0 << 12) // Multicast Offset - bits 47:36
#define RCTL_MO_35         (1 << 12) // Multicast Offset - bits 46:35
#define RCTL_MO_34         (2 << 12) // Multicast Offset - bits 45:34
#define RCTL_MO_32         (3 << 12) // Multicast Offset - bits 43:32
#define RCTL_BAM           (1 << 15) // Broadcast Accept Mode
#define RCTL_VFE           (1 << 18) // VLAN Filter Enable
#define RCTL_CFIEN         (1 << 19) // Canonical Form Indicator Enable
#define RCTL_CFI           (1 << 20) // Canonical Form Indicator Bit Value
#define RCTL_DPF           (1 << 22) // Discard Pause Frames
#define RCTL_PMCF          (1 << 23) // Pass MAC Control Frames
#define RCTL_SECRC         (1 << 26) // Strip Ethernet CRC

// Buffer Sizes
#define RCTL_BSIZE_256   (3 << 16)
#define RCTL_BSIZE_512   (2 << 16)
#define RCTL_BSIZE_1024  (1 << 16)
#define RCTL_BSIZE_2048  (0 << 16)
#define RCTL_BSIZE_4096  ((3 << 16) | (1 << 25))
#define RCTL_BSIZE_8192  ((2 << 16) | (1 << 25))
#define RCTL_BSIZE_16384 ((1 << 16) | (1 << 25))


// Transmit Command

#define CMD_EOP  (1 << 0) // End of Packet
#define CMD_IFCS (1 << 1) // Insert FCS
#define CMD_IC   (1 << 2) // Insert Checksum
#define CMD_RS   (1 << 3) // Report Status
#define CMD_RPS  (1 << 4) // Report Packet Sent
#define CMD_VLE  (1 << 6) // VLAN Packet Enable
#define CMD_IDE  (1 << 7) // Interrupt Delay Enable


// TCTL Register

#define TCTL_EN         (1 << 1)  // Transmit Enable
#define TCTL_PSP        (1 << 3)  // Pad Short Packets
#define TCTL_CT_SHIFT   4         // Collision Threshold
#define TCTL_COLD_SHIFT 12        // Collision Distance
#define TCTL_SWXOFF     (1 << 22) // Software XOFF Transmission
#define TCTL_RTLC       (1 << 24) // Re-transmit on Late Collision

#define TSTA_DD (1 << 0)          // Descriptor Done
#define TSTA_EC (1 << 1)          // Excess Collisions
#define TSTA_LC (1 << 2)          // Late Collision
#define LSTA_TU (1 << 3)          // Transmit Underrun

#define RX_DESC 16
#define TX_DESC 16

#define R_CTL      (0x00000)  /* Device Control Register - RW */
#define R_STATUS   (0x00008)  /* Device Status - R */
#define R_EERD     (0x00014)  /* EEPROM Read - RW */
#define R_ICR      (0x000C0)  /* Interrupt Cause Read - R */
#define R_ITR      (0x000C4)  /* Interrupt Throttling Register - RW */
#define R_IMS      (0x000D0)  /* Interrupt Mask Set - RW */
#define R_IMC      (0x000D8)  /* Interrupt Mask Clear - W */
#define R_RCTL     (0x00100)  /* RX Control - RW */
#define R_TCTL     (0x00400)  /* TX Control - RW */
#define R_TIPG     (0x00410)  /* TX Inter-packet gap -RW */
#define R_RDBAL    (0x02800)  /* RX Descriptor Base Address Low - RW */
#define R_RDBAH    (0x02804)  /* RX Descriptor Base Address High - RW */
#define R_RDLEN    (0x02808)  /* RX Descriptor Length - RW */
#define R_RDTR     (0x02820)  /* RX Delay Timer */
#define R_RADV     (0x0282C)  /* RX Interrupt Absolute Delay Timer */
#define R_RDH      (0x02810)  /* RX Descriptor Head - RW */
#define R_RDT      (0x02818)  /* RX Descriptor Tail - RW */
#define R_RDLEN    (0x02808)  /* RX Descriptor Length - RW */
#define R_RSRPD    (0x02C00)  /* RX Small Packet Detect Interrupt */
#define R_TDBAL    (0x03800)  /* TX Descriptor Base Address Low - RW */
#define R_TDBAH    (0x03804)  /* TX Descriptor Base Address High - RW */
#define R_TDLEN    (0x03808)  /* TX Descriptor Length - RW */
#define R_TDH      (0x03810)  /* TX Descriptor Head - RW */
#define R_TDT      (0x03818)  /* TX Descripotr Tail - RW */
#define R_MTA      (0x05200)  /* Multicast Table Array - RW Array */
#define R_RA       (0x05400)  /* Receive Address - RW Array */
#define R_RAL      (0x05400)  /* Receive Address Low */
#define R_RAH      (0x05404)  /* Receive Address High */

#define S_LU       (1 << 1)   /* Link Up Indication */

typedef struct _packed {
    u64 addr;
    u16 len;
    u16 cksum;
    u8 stat;
    u8 err;
    u16 special;
} rx_desc_t;

typedef struct _packed {
    u64 addr;
    u16 len;
    u8 cso;
    u8 cmd;
    u8 stat;
    u8 css;
    u16 special;
} tx_desc_t;

STATIC_ASSERT(sizeof(rx_desc_t) == 16, "wrong size");
STATIC_ASSERT(sizeof(tx_desc_t) == 16, "wrong size");

struct e1000;
typedef struct e1000 e1000_t;

typedef u32 (*read_op)(e1000_t *dev, u16 addr);
typedef void (*write_op)(e1000_t *dev, u16 addr, u32 val);

#include <textos/net.h>
#include <textos/dev/mbuf.h>

struct e1000 {
    nic_t nic;
    bool mmio;
    bool eeprom;
    bool e1000e;
    u8 irq;
    u64 base;
    u64 size;
    read_op read;
    write_op write;

    int txi, rxi;
    addr_t ptxds;
    addr_t prxds;
    tx_desc_t *txds; // tx descriptors
    rx_desc_t *rxds; // rx descriptors
    mbuf_t **rxb;
    mbuf_t **txb;
};

void mmio_reg_write(e1000_t *e, u16 addr, u32 val)
{
    (*((volatile u32 *)(e->base + addr))) = val;
}

u32 mmio_reg_read(e1000_t *e, u16 addr)
{
    return (*((volatile u32 *)(e->base + addr)));
}

void mmio_setup(e1000_t *e)
{
    vmap_map(e->base, __e1000_va,
        DIV_ROUND_UP(e->size, PAGE_SIZ),
        PE_PCD | PE_P | PE_RW, MAP_4K);
    e->base = __e1000_va;
}

void port_reg_write(e1000_t *dev, u16 addr, u32 val)
{
    outdw(dev->base, addr);
    outdw(dev->base + 4, val);
}

u32 port_reg_read(e1000_t *dev, u16 addr)
{
    outdw(dev->base, addr);
    return indw(dev->base + 4);
}

#define reg_set(addr, val) e->write(e, addr, val)
#define reg_get(addr) e->read(e, addr)

bool eeprom_detect(e1000_t *e)
{
    reg_set(R_EERD, 1);

    for (int i = 0 ; i < 1000 ; i++)
        if (reg_get(R_EERD) & 0x10)
            return true;

    return false;
}

u16 eeprom_read(e1000_t *e, u8 addr)
{
    u32 v32;
    if (e->eeprom)
    {
        reg_set(R_EERD, (1) | ((u32)addr << 8));
        while(!((v32 = reg_get(R_EERD)) & (1 << 4)));
    }
    else
    {
        reg_set(R_EERD, (1) | ((u32)(addr) << 2));
        while(!((v32 = reg_get(R_EERD)) & (1 << 1)));
    }
    return (u16)(v32 >> 16);
}

void mac_init(e1000_t *e)
{
    u16 *p = (u16 *)e->nic.mac;
    for (int i = 0 ; i < 3 ; i++)
        p[i] = eeprom_read(e, i);
}

void desc_alloc(addr_t *pa, void **desc, mbuf_t ***m, size_t num)
{
    size_t npg = DIV_ROUND_UP(num * 16, PAGE_SIZ);

    void *pp = pmm_allocpages(npg);
    *pa = (addr_t)pp;
    void *vp = vmm_allocvrt(npg);
    *desc = vp;
    vmap_map((addr_t)pp, (addr_t)vp, npg, PE_P | PE_RW, MAP_4K);
    memset(vp, 0, num * 16);

    *m = malloc(sizeof(mbuf_t *) * num);
}

void rx_init(e1000_t *e)
{
    // reg_set(R_RCTL, 0);

    // set Recv Addr Registers with the desired ethernet addresses
    // here we use the MAC addr provided by EEPROM if it has,
    // for QEMU, by EEPROM 52:54:00:12:34:56 is provided. actually
    // we can decide it by ourselves
    u64 mac;
    mac = *(u64 *)&e->nic.mac;
    mac &= 0xFFFFFFFFFF; // get low bits (0-5)
    mac |= (1ull << 63); // make addr valid
    reg_set(R_RAL, (u32)mac);
    reg_set(R_RAH, (u32)(mac >> 32));

    // initialize the MTA (Multicast Table Array) to 0b
    // MTA is a 4096-bit vector provided by ethernet controller
    for (int i = 0 ; i < 0x200 / 4 ; i += 4)
        reg_set(R_MTA + i, 0);

    // set the Intr Mask Set/Read (IMS) register to enable any interrupt
    // the software driver wants to be notified of when the event occurs
    // reg_set()
    
    // SEE: 3.2.7.1.1 Receive Interrupt Delay Timer / Packet Timer (RDTR)
    // no delay is used, enable immediate interrupts
    reg_set(R_RDTR, 0);
    reg_set(R_ITR, 0);
    
    // alloc some physical pages and map them to e->rxds
    desc_alloc(&e->prxds, (void **)&e->rxds, &e->rxb, RX_DESC);
    rx_desc_t *ptr = (rx_desc_t *)e->rxds;
    for (int i = 0 ; i < RX_DESC ; i++, ptr++)
    {
        mbuf_t *m = mbuf_alloc(0);
        ptr->addr = m->phy;
        e->rxb[i] = m;
    }

    reg_set(R_RDBAH, (u64)e->prxds >> 32);
    reg_set(R_RDBAL, (u64)e->prxds & 0xFFFFFFFF);
    reg_set(R_RDLEN, RX_DESC * sizeof(rx_desc_t));

    // set ptrs
    reg_set(R_RDH, 0);
    reg_set(R_RDT, RX_DESC - 1);

    e->rxi = 0;

    // set the Receive Control (RCTL) register
    u32 ctrl = 0;
    ctrl |= RCTL_EN;         // enable
    ctrl |= RCTL_SBP;        // save bad packet
    ctrl |= RCTL_UPE;        // 
    ctrl |= RCTL_MPE;        // 
    ctrl |= RCTL_LBM_NONE;   // disable loop
    ctrl |= RCTL_BAM;        // enable broadcast
    ctrl |= RCTL_SECRC;      // strip CRC
    ctrl |= RCTL_BSIZE_4096; // 4096-byte rx buffers
    reg_set(R_RCTL, ctrl);
}

void tx_init(e1000_t *e)
{
    // alloc some physical pages and map them to e->txds
    desc_alloc(&e->ptxds, (void **)&e->txds, &e->txb, TX_DESC);
    reg_set(R_TDBAH, (u64)e->ptxds >> 32);
    reg_set(R_TDBAL, (u64)e->ptxds & 0xFFFFFFFF);
    reg_set(R_TDLEN, TX_DESC * sizeof(tx_desc_t));

    e->txi = 0;

    // set ptrs
    reg_set(R_TDH, 0);
    reg_set(R_TDT, 0);

    // initialize the Transmit Ctrl Register (TCTL) for desired operation
    u32 ctrl = 0;
    ctrl |= TCTL_EN | TCTL_PSP;
    ctrl |= (0x10 << TCTL_CT_SHIFT);
    ctrl |= (0x40 << TCTL_COLD_SHIFT);
    reg_set(R_TCTL, ctrl);

    // set Inter Packet Gap (bit-time)
    u32 ipg = 0;
    ipg |= 10;         // IPGT
    ipg |= (10 << 10); // IPGR1
    ipg |= (10 << 20); // IPGR2
    // reg_set(R_TIPG, ipg);
}

#define IM_TXDW   (1 << 0) // Transmit Descriptor Written Back
#define IM_LSC    (1 << 2) // Link Status Change
#define IM_RXO    (1 << 6) // Receiver FIFO Overrun
#define IM_RXT0   (1 << 7) // Receiver Timer Interrupt

void intr_init(e1000_t *e)
{
    u32 mask = 0;
    mask |= IM_LSC;
    mask |= IM_RXO;
    mask |= IM_RXT0;

    reg_set(R_IMS, mask);
}

void soft_reset(e1000_t *e)
{
    u32 bit = (1 << 26);
    u32 ctrl = reg_get(R_CTL);
    reg_set(R_CTL, ctrl | bit);
    do {
        ctrl = reg_get(R_CTL);
    } while (ctrl & bit);
}

int send_packet(e1000_t *e, mbuf_t *m)
{
    tx_desc_t *tx = &e->txds[e->txi];
    tx->addr = m->phy;
    tx->len = m->len;
    tx->cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    tx->stat = 0;

    int prev = e->txi;
    e->txi = (prev + 1) % TX_DESC;
    reg_set(R_TDT, e->txi);
    while (!(e->txds[prev].stat & 0xff));

    return 0;
}

#define RXD_DD  0x1 // descriptor done
#define RXD_EOP 0x2 // end of packets

int recv_packet(e1000_t *e)
{
    while (true)
    {
        // deliver all packets arrived
        int i = e->rxi;
        rx_desc_t *rx = &e->rxds[i];
        if (!(rx->stat & RXD_DD))
            break;
        e->rxb[i]->len = e->rxds[i].len;
        nic_eth_rx(&e->nic, e->rxb[i]);

        // new buffer
        e->rxb[i] = mbuf_alloc(0);
        rx->addr = e->rxb[i]->phy;
        rx->stat = 0;
        reg_set(R_RDT, i);
        e->rxi = (e->rxi + 1) % RX_DESC;
    }
    return 0;
}

e1000_t e1000;

void e1000_test(e1000_t *e)
{
    mbuf_t *m = mbuf_alloc(MBUF_DEFROOM);
    int len;

    char load[] = "hello world!!!";
    len = strlen(load);
    memcpy(mbuf_put(m, len), load, len);
    
    ethhdr_t ef = {
        { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
        { 0x52, 0x54, 0x00, 0x12, 0x34, 0x56 },
        htons(0x0800)
    };
    len = sizeof(ethhdr_t);
    memcpy(mbuf_pull(m, len), &ef, len);
    send_packet(e, m);
}

__INTR_HANDLER(e1000_handler)
{
    e1000_t *e = &e1000;

    u32 stat = reg_get(R_STATUS);

    if (stat & IM_LSC)
    {
        PANIC("link status changed\n");
    }

    if (stat & IM_RXO)
    {
        PANIC("recv ring overran\n");
    }

    if (stat & IM_RXT0)
    {
        recv_packet(e);
    }

    reg_set(R_ICR, 0xFFFFFFFF);
    lapic_sendeoi();
}

#include <lai/helpers/pci.h>

void irq_init(e1000_t *e)
{
    intr_register(INT_E1000, e1000_handler);
    if (e->e1000e)
    {
        if (pci_set_msi(e->nic.pi, INT_E1000) < 0)
            PANIC("cannot set MSI\n");
    }
    else
    {
        pci_idx_t *idx = e->nic.pi;
        acpi_resource_t ar;

        ASSERTK(lai_pci_route_pin(
            &ar, 0,
            idx->bus,
            idx->slot,
            idx->func,
            pci_get_pin(idx)) == 0);

        e->irq = ar.base;
        ioapic_rteset(e->irq, _IOAPIC_RTE(INT_E1000));
    }
}

pci_idx_t *e1000_find(e1000_t *e)
{
    e->e1000e = false;

    pci_idx_t *idx;
    idx = pci_find(INTEL_VEND, E1000_EMU, 0);
    if (idx)
        return idx;

    idx = pci_find(INTEL_VEND, E1000E_EMU, 0);
    if (idx)
        e->e1000e = true;
    return idx;
}

void e1000_send(nic_t *n, mbuf_t *m)
{
    e1000_t *e = CR(n, e1000_t, nic);
    send_packet(e, m);
}

extern void arp_request(ipv4_t dip);

/*
 * it is not a good idea to use pci driver to test e1000e, because
 * we do not use the extended config space, we use old pci interface
 * to have a test on MSI (Message Signaled Interrupts)!
 * And later we will cope with it using `lai` to support ioapic int for NIC
 */
void e1000_init()
{
    e1000_t *e = &e1000;

    pci_idx_t *idx = e1000_find(e);
    if (idx == NULL)
        PANIC("e1000 not found\n");

    pci_bar_t bar0;
    pci_get_bar(idx, &bar0, 0);
    e->nic.pi = idx;
    e->base = bar0.base;
    e->size = bar0.size;
    if (!bar0.mmio)
    {
        e->mmio = false;
        e->read = port_reg_read;
        e->write = port_reg_write;
    }
    else
    {
        e->mmio = true;
        e->read = mmio_reg_read;
        e->write = mmio_reg_write;
        mmio_setup(e);
    }
    
    e->eeprom = eeprom_detect(e);
    DEBUGK(K_INIT,
      "E1000 - mmio = %d, eeprom = %d, e1000e = %d\n",
      e->mmio, e->eeprom, e->e1000e);

    irq_init(e);
    
    pci_set_busmaster(idx);

    mac_init(e);
    DEBUGK(K_INIT,
      "E1000 - MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
      e->nic.mac[0], e->nic.mac[1], e->nic.mac[2],
      e->nic.mac[3], e->nic.mac[4], e->nic.mac[5]);

    rx_init(e);
    tx_init(e);
    intr_init(e);

    // default ip
    ipv4_t qemu_ip = { 192, 168, 2, 2 };
    memcpy(e->nic.ip, &qemu_ip, sizeof(ipv4_t));

    e->nic.send = e1000_send;
    e->nic.link = (reg_get(R_STATUS) & S_LU) == S_LU;
    ASSERTK(e->nic.link == true);

    e1000_test(e);

    nic0 = &e->nic;

    ipv4_t dip = { 192, 168, 2, 1 };
    arp_request(dip);
}

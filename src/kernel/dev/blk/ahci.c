// AHCI (Advance Host Controller Interface)
// All page number refers to Spec, Rev. 1.3.1: Serial ATA AHCI
#include <textos/mm/vmm.h>
#include <textos/mm/map.h>
#include <textos/mm/heap.h>
#include <textos/dev/pci.h>
#include <textos/dev/buffer.h>
#include <textos/klib/list.h>
#include <textos/klib/string.h>

#define cutl(x) ((u32)((u64)x & 0xffffffff))
#define cuth(x) (cutl((u64)x >> 32))

// FIS - Frame Information Structure
typedef enum
{
    FIS_TYPE_REG_H2D = 0x27,   // Register FIS - host to device
    FIS_TYPE_REG_D2H = 0x34,   // Register FIS - device to host
    FIS_TYPE_DMA_ACT = 0x39,   // DMA activate FIS - device to host
    FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional
    FIS_TYPE_DATA = 0x46,      // Data FIS - bidirectional
    FIS_TYPE_BIST = 0x58,      // BIST activate FIS - bidirectional
    FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host
    FIS_TYPE_DEV_BITS = 0xA1,  // Set device bits FIS - device to host
} fis_type_t;

struct fis_reg_h2d
{
    // DWORD 0
    uint8_t fis_type;   // FIS_TYPE_REG_H2D
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 3;   // Reserved
    uint8_t c : 1;      // 1: Command, 0: Control
    uint8_t command;    // Command register
    uint8_t featurel;   // Feature register, 7:0

    // DWORD 1
    uint8_t lba0;   // LBA low register, 7:0
    uint8_t lba1;   // LBA mid register, 15:8
    uint8_t lba2;   // LBA high register, 23:16
    uint8_t device; // ATA Device register (SEE ide.c)

    // DWORD 2
    uint8_t lba3;     // LBA register, 31:24
    uint8_t lba4;     // LBA register, 39:32
    uint8_t lba5;     // LBA register, 47:40
    uint8_t featureh; // Feature register, 15:8

    // DWORD 3
    uint8_t countl;  // Count register, 7:0
    uint8_t counth;  // Count register, 15:8
    uint8_t icc;     // Isochronous command completion
    uint8_t control; // Control register

    // DWORD 4
    uint8_t rsv1[4]; // Reserved
} _packed;

struct reg_d2h
{
    // DWORD 0
    uint8_t fis_type;   // FIS_TYPE_REG_D2H
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 2;   // Reserved
    uint8_t i : 1;      // Interrupt bit
    uint8_t rsv1 : 1;   // Reserved
    uint8_t status;     // Status register
    uint8_t error;      // Error register

    // DWORD 1
    uint8_t lba0;   // LBA low register, 7:0
    uint8_t lba1;   // LBA mid register, 15:8
    uint8_t lba2;   // LBA high register, 23:16
    uint8_t device; // Device register

    // DWORD 2
    uint8_t lba3; // LBA register, 31:24
    uint8_t lba4; // LBA register, 39:32
    uint8_t lba5; // LBA register, 47:40
    uint8_t rsv2; // Reserved

    // DWORD 3
    uint8_t countl;  // Count register, 7:0
    uint8_t counth;  // Count register, 15:8
    uint8_t rsv3[2]; // Reserved

    // DWORD 4
    uint8_t rsv4[4]; // Reserved
} _packed;

struct data
{
    // DWORD 0
    uint8_t fis_type;   // FIS_TYPE_DATA
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 4;   // Reserved
    uint8_t rsv1[2];    // Reserved
    // DWORD 1 ~ N
    uint32_t data[1]; // Payload
} _packed;

struct pio_setup
{
    // DWORD 0
    uint8_t fis_type;   // FIS_TYPE_PIO_SETUP
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 1;   // Reserved
    uint8_t d : 1;      // Data transfer direction, 1 - device to host
    uint8_t i : 1;      // Interrupt bit
    uint8_t rsv1 : 1;
    uint8_t status; // Status register
    uint8_t error;  // Error register

    // DWORD 1
    uint8_t lba0;   // LBA low register, 7:0
    uint8_t lba1;   // LBA mid register, 15:8
    uint8_t lba2;   // LBA high register, 23:16
    uint8_t device; // Device register

    // DWORD 2
    uint8_t lba3; // LBA register, 31:24
    uint8_t lba4; // LBA register, 39:32
    uint8_t lba5; // LBA register, 47:40
    uint8_t rsv2; // Reserved

    // DWORD 3
    uint8_t countl;   // Count register, 7:0
    uint8_t counth;   // Count register, 15:8
    uint8_t rsv3;     // Reserved
    uint8_t e_status; // New value of status register

    // DWORD 4
    uint16_t tc;     // Transfer count
    uint8_t rsv4[2]; // Reserved
} _packed;

struct dma_setup
{
    // DWORD 0
    uint8_t fis_type;   // FIS_TYPE_DMA_SETUP
    uint8_t pmport : 4; // Port multiplier
    uint8_t rsv0 : 1;   // Reserved
    uint8_t d : 1;      // Data transfer direction, 1 - device to host
    uint8_t i : 1;      // Interrupt bit
    uint8_t a : 1;      // Auto-activate. Specifies if need DMA Activate FIS
    uint8_t rsv1[2];    // Reserved

    // DWORD 1 & 2
    uint64_t bufid; // DMA Buffer Identifier. Used to Identify DMA buffer
                    // in host memory. SATA Spec says host specific and
                    // not in Spec. Trying AHCI spec might work.

    uint32_t rsvd;   // More reserved
    uint32_t bufoff; // Byte offset into buffer. First 2 bits must be 0
    uint32_t tc;     // Number of bytes to transfer. Bit 0 must be 0
    uint32_t rsv2;   // Reserved
} _packed;

/*
 * organization work flow:
 * ahci0
 *   - port0
 *     - cmd hdr
 *   - port1
 *   - port2
 */
struct hba_port
{
    uint32_t clb;      // 0x00, command list base address, 1K-byte aligned
    uint32_t clbu;     // 0x04, command list base address upper 32 bits
    uint32_t fb;       // 0x08, FIS base address, 256-byte aligned
    uint32_t fbu;      // 0x0C, FIS base address upper 32 bits
    uint32_t is;       // 0x10, interrupt status
    uint32_t ie;       // 0x14, interrupt enable
    uint32_t cmd;      // 0x18, command and status
    uint32_t rsv0;     // 0x1C, Reserved
    uint32_t tfd;      // 0x20, task file data
    uint32_t sig;      // 0x24, signature
    uint32_t ssts;     // 0x28, SATA status (SCR0:SStatus)
    uint32_t sctl;     // 0x2C, SATA control (SCR2:SControl)
    uint32_t serr;     // 0x30, SATA error (SCR1:SError)
    uint32_t sact;     // 0x34, SATA active (SCR3:SActive)
    uint32_t ci;       // 0x38, command issue
    uint32_t sntf;     // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t fbs;      // 0x40, FIS-based switch control
    uint32_t rsv1[11]; // 0x44 ~ 0x6F, Reserved
    uint32_t vend[4];  // 0x70 ~ 0x7F, vendor specific
} _packed;

// SEE 3.3.8 Offset 20h: PxTFD – Port x Task File Data
#define ATA_CB_STAT_BSY  0x80 // busy
#define ATA_CB_STAT_DRQ  0x08 // data request
#define ATA_CB_STAT_ERR  0x01 // error (ATA)

struct hba_mem
{
    // 0x00 - 0x2B, Generic Host Control
    uint32_t cap;     // 0x00, Host capability
    uint32_t ghc;     // 0x04, Global host control
    uint32_t is;      // 0x08, Interrupt status
    uint32_t pi;      // 0x0C, Port implemented
    uint32_t vs;      // 0x10, Version
    uint32_t ccc_ctl; // 0x14, Command completion coalescing control
    uint32_t ccc_pts; // 0x18, Command completion coalescing ports
    uint32_t em_loc;  // 0x1C, Enclosure management location
    uint32_t em_ctl;  // 0x20, Enclosure management control
    uint32_t cap2;    // 0x24, Host capabilities extended
    uint32_t bohc;    // 0x28, BIOS/OS handoff control and status

    // 0x2C - 0x9F, Reserved
    uint8_t rsv[0xA0 - 0x2C];
    // 0xA0 - 0xFF, Vendor specific registers
    uint8_t vend[0x100 - 0xA0];
    // 0x100 - 0x10FF, Port control registers
    volatile struct hba_port ports[1]; // 1 ~ 32
} _packed;

struct cmd_hdr
{
    // DW0
    uint8_t cfl : 5;  // Command FIS length in DWORDS, 2 ~ 16
    uint8_t a : 1;    // ATAPI
    uint8_t w : 1;    // Write, 1: H2D, 0: D2H
    uint8_t p : 1;    // Prefetchable
    uint8_t r : 1;    // Reset
    uint8_t b : 1;    // BIST
    uint8_t c : 1;    // Clear busy upon R_OK
    uint8_t rsv0 : 1; // Reserved
    uint8_t pmp : 4;  // Port multiplier port
    uint16_t prdtl;   // Physical region descriptor table length in entries

    // DW1
    uint32_t prdbc;   // Physical region descriptor byte count transferred
    uint32_t ctba;    // Command table descriptor base address
    uint32_t ctbau;   // Command table descriptor base address upper 32 bits
    uint32_t rsv1[4]; // Reserved
};

struct prdt_entry
{
    uint32_t dba;  // Data base address
    uint32_t dbau; // Data base address upper 32 bits
    uint32_t rsv0; // Reserved

    // DW3
    uint32_t dbc : 22; // Byte count, 4M max
    uint32_t rsv1 : 9; // Reserved
    uint32_t i : 1;    // Interrupt on completion
};

struct cmd_tbl
{
    uint8_t cfis[64]; // Command FIS
    uint8_t acmd[16]; // ATAPI command, 12 or 16 bytes
    uint8_t rsv[48];  // Reserved
    struct prdt_entry
        prdt_entry[248]; // Physical region descriptor table entries, 0 ~ 65535
};

#define ATA_CMD_IDENTIFY 0xEC

struct ahci
{
    uint nrport;
    list_t l_port;
    addr_t _free_clb_phy;
    addr_t _free_fis_phy;
    addr_t _free_clb;
    addr_t _free_fis;
    volatile struct hba_mem *mem;
};

typedef enum ahci_type
{
    AHCI_DEV_NULL = 0,
    AHCI_DEV_SATA = 1,
    AHCI_DEV_SEMB = 2,
    AHCI_DEV_PM = 3,
    AHCI_DEV_SATAPI = 4,
} ahci_type_t;

struct port
{
    // gained by scanning
    int idx;
    ahci_type_t type;
    struct ahci *ahci;
    volatile struct hba_port *hp;
    list_t l_port;

    char serial_num[21];
    char model_num[41];

    // initialized by init_port
    void *fis;
    struct cmd_hdr *clb;      // array of cmd_hdr of this port
    struct cmd_tbl *ctbl[32]; // array of pointers to the cmd_tbl of slot i
};

#define HBA_CLB_SIZE  1024
#define HBA_FIS_SIZE  256
#define HBA_CTBA_SIZE sizeof(struct cmd_tbl)

#define SATA_SIG_ATA   0x00000101 // SATA drive
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive
#define SATA_SIG_SEMB  0xC33C0101 // Enclosure management bridge
#define SATA_SIG_PM    0x96690101 // Port multiplier

#define HBA_PORT_DET_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE  0x1

#define PxCMD_ST       (1 << 0)    // Start cmd engine
#define PxCMD_FRE      (1 << 4)    // FIS Receive Enable
#define PxCMD_FR       (1 << 14)   // FIS Receive Running (RO)
#define PxCMD_CR       (1 << 15)   // Command List Running (RO)
#define PxCMD_CPD      (1 << 20)   // Cold Presence Detect
#define PxCMD_ICC_MASK (0xF << 28) // Interface Communication Control

static void alloc_clb(struct ahci *a, addr_t *pa, struct cmd_hdr **va)
{
    if (!(a->_free_clb & PAGE_MASK))
        vmm_allocpv(1, &a->_free_clb, &a->_free_clb_phy);
    *va = (void *)a->_free_clb;
    *pa = a->_free_clb_phy;
    a->_free_clb += HBA_CLB_SIZE;
    a->_free_clb_phy += HBA_CLB_SIZE;
}

static void alloc_fis(struct ahci *a, addr_t *pa, void **va)
{
    if (!(a->_free_fis & PAGE_MASK))
        vmm_allocpv(1, &a->_free_fis, &a->_free_fis_phy);
    *va = (void *)a->_free_fis;
    *pa = a->_free_fis_phy;
    a->_free_fis += HBA_FIS_SIZE;
    a->_free_fis_phy += HBA_FIS_SIZE;
}

static void alloc_ctba(addr_t *pa, struct cmd_tbl **va)
{
    vmm_allocpv(1, (addr_t *)va, pa);
}

static void stop_port(volatile struct hba_port *hp)
{
    hp->cmd &= ~(PxCMD_ST | PxCMD_FRE);
    for (;;) {
        if (hp->cmd & PxCMD_FR) continue;
        if (hp->cmd & PxCMD_CR) continue;
        break;
    }
}

static void start_port(volatile struct hba_port *hp)
{
    while (hp->cmd & PxCMD_CR)
        ;
    hp->cmd |= PxCMD_ST | PxCMD_FRE;
}

// mutex
static int get_port_slot(volatile struct hba_port *hp)
{
    u32 taken = hp->sact | hp->ci;
    u32 avail = ~taken;
    for (int i = 0; i < 32; i++, avail >>= 1) {
        if (avail & 1) return i;
    }
    return -1;
}

static void ahci_ident_port(struct port *port)
{
    volatile struct hba_port *hp = port->hp;
    int slot = get_port_slot(hp);
    buffer_t *bdma = bdma_alloc(512);
    struct cmd_hdr *hdr = &port->clb[slot];
    struct fis_reg_h2d *fis = (struct fis_reg_h2d *)port->ctbl[slot]->cfis;
    struct prdt_entry *prdt = (struct prdt_entry *)&port->ctbl[slot]->prdt_entry[0];
    prdt->dbc = bdma->siz - 1;
    prdt->dba = cutl(bdma->phy);
    prdt->dbau = cuth(bdma->phy);
    prdt->i = 0;
    hdr->prdtl = 1;
    memset(fis, 0, sizeof(struct fis_reg_h2d));
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->countl = 1;
    fis->counth = 0;
    fis->command = ATA_CMD_IDENTIFY;
    fis->device = 0x40; // lba mode
    fis->c = 1;         // send to cmd reg
    // wait for sending a new cmd
    while (hp->tfd & (ATA_CB_STAT_BSY | ATA_CB_STAT_DRQ))
        ;
    hp->ci |= 1 << slot;

    u16 *buf = bdma->blk;
    for (int i = 0; i < 10; i++) {
        port->serial_num[i * 2] = buf[10 + i] >> 8;
        port->serial_num[i * 2 + 1] = buf[10 + i] & 0xFF;
    }
    for (int i = 0; i < 20; i++) {
        port->model_num[i * 2] = buf[27 + i] >> 8;
        port->model_num[i * 2 + 1] = buf[27 + i] & 0xFF;
    }
    DEBUGK(K_INFO, "AHCI port %d info:\n", port->idx);
    DEBUGK(K_INFO | K_CONT, "disk sn : %s\n", port->serial_num);
    DEBUGK(K_INFO | K_CONT, "disk model : %s\n", port->model_num);

    bdma_free(bdma);
}

static void ahci_init_port(struct port *port)
{
    addr_t pa;
    stop_port(port->hp);
    // allocate command_list (32 bytes x32) memory
    alloc_clb(port->ahci, &pa, &port->clb);
    port->hp->clb = cutl(pa);
    port->hp->clbu = cuth(pa);
    memset(port->clb, 0, HBA_CLB_SIZE);

    // allocate FIS (256 bytes x1) memory
    alloc_fis(port->ahci, &pa, &port->fis);
    port->hp->fb = cutl(pa);
    port->hp->fbu = cuth(pa);
    memset(port->fis, 0, HBA_FIS_SIZE);

    for (int i = 0; i < 32; i++) {
        struct cmd_hdr *hdr = port->clb + i;
        alloc_ctba(&pa, &port->ctbl[i]);
        hdr->prdtl = 1;
        hdr->ctba = cutl(pa);
        hdr->ctbau = cuth(pa);
    }
    start_port(port->hp);
}

static void ahci_init_ports(struct ahci *ahci)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &ahci->l_port)
    {
        struct port *port = CR(ptr, struct port, l_port);
        ahci_init_port(port);
        ahci_ident_port(port);
    }
}

static ahci_type_t ahci_probe_drv(volatile struct hba_port *hp)
{
    u32 ssts = hp->ssts;
    u8 ipm = (ssts >> 8) & 0xf;
    u8 det = ssts & 0xf;
    if (det != HBA_PORT_DET_PRESENT) return AHCI_DEV_NULL; // status
    if (ipm != HBA_PORT_IPM_ACTIVE) return AHCI_DEV_NULL;  // power
    switch (hp->sig) {
    case SATA_SIG_ATAPI:
        return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
        return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
        return AHCI_DEV_PM;
    default:
        return AHCI_DEV_SATA;
    }
}

static void ahci_probe_port(struct ahci *ahci)
{
    static const char *type_str[] = {
        [AHCI_DEV_NULL] = "none / poweroff",
        [AHCI_DEV_SATA] = "sata",
        [AHCI_DEV_SEMB] = "semb",
        [AHCI_DEV_PM] = "pm",
        [AHCI_DEV_SATAPI] = "satapi",
    };
    u32 pi = ahci->mem->pi;
    for (int i = 0; i < 32; i++, pi >>= 1) {
        if (!(pi & 1)) continue;
        volatile struct hba_port *hp = ahci->mem->ports + i;
        ahci_type_t type = ahci_probe_drv(hp);
        DEBUGK(K_INFO, "AHCI PORT[#%d] - drive: %s\n", i, type_str[type]);
        if (type != AHCI_DEV_NULL) {
            struct port *port = malloc(sizeof(struct port));
            port->idx = i;
            port->type = type;
            port->ahci = ahci;
            port->hp = hp;
            list_pushback(&ahci->l_port, &port->l_port);
            ahci->nrport++;
        }
    }
    DEBUGK(K_INFO, "AHCI total %d port(s) found\n", ahci->nrport);
}

static void ahci_init_ctrl(struct ahci *ahci, pci_idx_t *pi)
{
    {
        pci_bar_t abar;
        pci_get_bar(pi, &abar, 5);
        DEBUGK(K_INFO, "abar.mmio = %d\n", abar.mmio);
        DEBUGK(K_INFO, "abar.base = %lx\n", abar.base);
        addr_t pa = (addr_t)abar.base;
        addr_t va = (addr_t)vmm_allocvrt(1);
        vmap_map(pa, va, DIV_ROUND_UP(sizeof(struct hba_mem), PAGE_SIZE),
                 PE_P | PE_RW | PE_PCD);
        ahci->mem = (struct hba_mem *)va;
    }
    if (!ahci->mem) return;

    int ncs = (ahci->mem->cap >> 8) & 0x1f;
    DEBUGK(K_INFO, "AHCI max slot number = %d\n", ncs + 1);

    ahci->nrport = 0;
    list_init(&ahci->l_port);
    ahci->_free_clb_phy = 0;
    ahci->_free_fis_phy = 0;
    ahci->_free_clb = 0;
    ahci->_free_fis = 0;
    // connect to hardware
    ahci_probe_port(ahci);
    ahci_init_ports(ahci);
}

struct ahci __ahci[1];

void ahci_init()
{
    pci_idx_t *pi = pci_find_class(0x0106, 0);
    if (!pi) {
        DEBUGK(K_INFO, "AHCI not supported\n");
        return;
    }
    ahci_init_ctrl(&__ahci[0], pi);
}

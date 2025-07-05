#include <io.h>
#include <irq.h>
#include <intr.h>
#include <textos/dev.h>
#include <textos/dev/pci.h>
#include <textos/task.h>
#include <textos/panic.h>
#include <textos/mm.h>
#include <textos/mm/map.h>
#include <textos/printk.h>

#include <string.h>

#define SECT_SIZ 512

#define BASE_PRI 0x1F0 // Prime
#define BASE_SEC 0x170 // Secondary

#define R_DATA (0) // 
#define R_ERRF (1) // Error or features
#define R_SECC (2) // Sector count
#define R_LBAL (3) // LBA low
#define R_LBAM (4) // LBA mid
#define R_LBAH (5) // LBA high
#define R_DEV  (6) // Device
#define R_SCMD (7) // Status or cmd

#define R_BCTL 0x206 // Offset of the block register

typedef struct
{
    int wait;
} channel_t;

typedef struct _packed __attribute__((aligned(4)))
{
    u32 base; // physical
    u16 size; // in bytes
    u16 flag; // EOT
} ideprd_t;

static channel_t channel[2]; // 2 channels

typedef struct {
    bool dma;
    u16 iobase;
    u16 bmbase;
    u8 dev;
    ideprd_t prd;
    channel_t *channel;
    char serial_num[21];
    char model_num[41];
} ide_t;

static ide_t ide[4];

#define STAT_ERR   (1 << 0) // Error
#define STAT_IDX   (1 << 2) // Index
#define STAT_CD    (1 << 3) // Corrected data
#define STAT_RQRDY (1 << 4) // Data request ready
#define STAT_WF    (1 << 5) // Drive write fault
#define STAT_RDY   (1 << 6) // Drive ready
#define STAT_BSY   (1 << 7) // Drive Busy

//

#define DEV_MASTER (0 << 4)
#define DEV_SLAVE  (1 << 4)

#define DEV_CHS (0 << 6)
#define DEV_LBA (1 << 6)

/* Macro to set the device register, and
   the last binary to set bits which are always `1` */
#define SET_DEV(opt) ((u8)(opt | DEV_LBA | 0b10100000))

#define CMD_IDENT 0xE0
#define CMD_READ  0x20
#define CMD_WRITE 0x30

#define CMD_RDMA  0xC8 // lba 28
#define CMD_WDMA  0xCA // lba 28 

#if CONFIG_IDE_USE_INTR

/* TODO: Lock device */

__INTR_HANDLER(ide_handler)
{
    lapic_sendeoi();

    int idx = (vector - INT_PRIDISK);
    channel_t *stat = &channel[idx];

    if (stat->wait < 0)
        return ;

    DEBUGK(K_DEV, "Read opt has finished, waking proc... -> %d\n", stat->wait);
    task_unblock(stat->wait);
    stat->wait = -1;
}

#endif

static void read_sector(u16 port, u16 *data)
{
    __asm__ volatile (
        "cld\n"
        "rep insw\n" 
        : : "c"(256), "d"(port + R_DATA), "D"(data)
        : "memory"
        );
}

static void write_sector(u16 port, u16 *data)
{
    __asm__ volatile (
        "cld\n"
        "rep outsw\n" 
        : : "c"(256), "d"(port + R_DATA), "D"(data)
        : "memory"
        );
}

static void ide_select(ide_t *pri, u32 lba, u8 cnt)
{
    outb(pri->iobase + R_DEV,  SET_DEV(pri->dev) | (lba >> 24)); // 选择设备并发送 LBA 的高4位
    outb(pri->iobase + R_SECC, cnt);                // 扇区数目
    outb(pri->iobase + R_LBAL, lba & 0xFF);         // LBA 0_7
    outb(pri->iobase + R_LBAM, (lba >> 8) & 0xFF);  // LBA 8_15
    outb(pri->iobase + R_LBAH, (lba >> 16) & 0xFF); // LBA 16_23
}

static void ide_waitrw(ide_t *pri)
{
    while (inb(pri->iobase + R_SCMD) & STAT_BSY) ;
}

static void ide_block(ide_t *pri)
{
#if CONFIG_IDE_USE_INTR
    pri->channel->wait = task_current()->pid;
    task_block();
#else
    ide_waitrw(pri);
#endif
}

#define R_BMCMD  0
#define R_BMSTAT 2
#define R_BMPPRD 4

#define CMD_BMSTOP  (0 << 0)
#define CMD_BMSTART (1 << 0)
#define CMD_BMREAD  (1 << 3)
#define CMD_BMWRITE (0 << 3)

#define STAT_BMACT  (1 << 0)
#define STAT_BMERR  (1 << 1)
#define STAT_BMINT  (1 << 2)

#include <textos/assert.h>

static void ide_setdma(ide_t *pri, void *buf, u8 cmd, uint len)
{
    /*
     * 缓冲区在物理空间上必须是连续的,
     * 保证它不跨页是一个临时的措施 (可能临时吧!)
     */
    ASSERTK(((addr_t)buf + len) <= ((addr_t)buf &~ PAGE_MASK) + PAGE_SIZ);

    addr_t va = (addr_t)buf;
    addr_t pa = vmap_query(va);
    addr_t pprd = vmap_query((addr_t)&pri->prd);

    pri->prd.base = pa;
    pri->prd.size = len;
    pri->prd.flag = (1 << 15);

    outdw(pri->bmbase + R_BMPPRD, pprd);  // prd addr
    outb(pri->bmbase + R_BMCMD, cmd);     // direction

    u8 status = inb(pri->bmbase + R_BMSTAT);
    status |= STAT_BMERR | STAT_BMINT;
    outb(pri->bmbase + R_BMSTAT, status); // set
}

static void ide_rundma(ide_t *pri, u8 rw)
{
    outb(pri->iobase + R_SCMD, rw);

    u8 bmcmd = inb(pri->bmbase + R_BMCMD);
    outb(pri->bmbase + R_BMCMD, bmcmd | CMD_BMSTART);
}

static void ide_enddma(ide_t *pri)
{
    u8 bmcmd = inb(pri->bmbase + R_BMCMD);
    outb(pri->bmbase + R_BMCMD, bmcmd & ~CMD_BMSTART);
    
    u8 status = inb(pri->bmbase + R_BMSTAT);
    status |= STAT_BMERR | STAT_BMINT;
    outb(pri->bmbase + R_BMSTAT, status); // set
}

static void ide_runpio(ide_t *pri, u8 rw)
{
    outb(pri->iobase + R_SCMD, rw);
}

static void ide_dma_read(devst_t *dev, u32 lba, void *data, u8 cnt)
{
    UNINTR_AREA_START();

    ide_t *pri = dev->pdata;
    lba &= 0xFFFFFFF;
    ide_waitrw(pri);
    
    ide_setdma(pri, data, CMD_BMREAD, cnt * SECT_SIZ);
    ide_select(pri, lba, cnt);
    ide_rundma(pri, CMD_RDMA);
    ide_block(pri);
    ide_enddma(pri);
    
    UNINTR_AREA_END();
}

static void ide_dma_write(devst_t *dev, u32 lba, void *data, u8 cnt)
{
    UNINTR_AREA_START();

    ide_t *pri = dev->pdata;
    lba &= 0xFFFFFFF;
    ide_waitrw(pri);

    ide_setdma(pri, data, CMD_BMWRITE, cnt * SECT_SIZ);
    ide_select(pri, lba, cnt);
    ide_rundma(pri, CMD_WDMA);
    
    UNINTR_AREA_END();
}

static void ide_pio_read(devst_t *dev, u32 lba, void *data, u8 cnt)
{
    UNINTR_AREA_START();

    ide_t *pri = dev->pdata;
    lba &= 0xFFFFFFF;
    ide_waitrw(pri);

    ide_select(pri, lba, cnt);
    ide_runpio(pri, CMD_READ);

    ide_block(pri);
    for (int i = 0 ; i < cnt ; i++) {
        read_sector(pri->iobase, data);
        data += SECT_SIZ;
    }
    
    UNINTR_AREA_END();
}

static void ide_pio_write(devst_t *dev, u32 lba, void *data, u8 cnt)
{
    UNINTR_AREA_START();

    ide_t *pri = dev->pdata;
    lba &= 0xFFFFFFF;
    ide_waitrw(pri);

    ide_select(pri, lba, cnt);
    ide_runpio(pri, CMD_WRITE);

    for (int i = 0 ; i < cnt ; i++) {
        write_sector(pri->iobase, data);
        data += SECT_SIZ;
        ide_block(pri);
    }
    
    UNINTR_AREA_END();
}

#include <textos/args.h>
#include <textos/klib/vsprintf.h>

static void ide_mkname(devst_t *dev, char res[32], int nr)
{
    sprintf(res, "%s%d", dev->name, nr);
}

#define ID_CONFIG   0   // Bit flags                  1  (word)
#define ID_SN       10  // Serial number              10 (word)
#define ID_FWVER    23  // Firmware version           8  (word)
#define ID_MODEL    27  // Model number               20 (word)
#define ID_SUPPORT  49  // Capabality                 1  (byte)
#define ID_ADDR_28  60  // 28位可寻址的全部逻辑扇区数     2  (word)
#define ID_ADDR_48  100 // 28位可寻址的全部逻辑扇区数     3  (word)
#define ID_MSN      176 // Current media sn           60 (word)

static bool ide_identify(ide_t *pri)
{
    outb(pri->iobase + R_DEV , SET_DEV(pri->dev));
    outb(pri->iobase + R_SCMD, 0xEC);

    for (int i = 0 ; i < 0xFFFF ; i++) ;

    u8 stat = inb(pri->iobase + R_SCMD);
    if (stat == 0 || stat & STAT_ERR) // 没有这个设备, 或者错误
        return false;                 // 中断识别

    u16 buf[256];
    read_sector(pri->iobase, buf);
    
    for (int i = 0 ; i < 10 ; i++)
    {
        pri->serial_num[i*2  ] = buf[ID_SN + i] >> 8;
        pri->serial_num[i*2+1] = buf[ID_SN + i] &  0xFF;
    }
    for (int i = 0 ; i < 20 ; i++)
    {
        pri->model_num[i*2  ] = buf[ID_MODEL + i] >> 8;
        pri->model_num[i*2+1] = buf[ID_MODEL + i] &  0xFF;
    }

    return true;
}

static char *ideid[] = {
    "hda",
    "hdb",
    "hdc",
    "hdd",
};

static void init(int x, u16 bmbase)
{
#if CONFIG_IDE_NO_DMA
    bmbase = 0;
#endif
    bool dma = bmbase != 0;
    ide_t *pri = &ide[x];

    pri->dma = dma;
    pri->channel = &channel[x / 2];
    pri->dev = x & 1 ? DEV_SLAVE : DEV_MASTER;
    if (x < 2)
    {
        pri->iobase = BASE_PRI;
        pri->bmbase = bmbase;
    }
    else
    {
        pri->iobase = BASE_SEC;
        pri->bmbase = bmbase + 8;
    }
    
    if (!ide_identify(pri))
        return ;

    devst_t *dev = dev_new();
    dev->name = ideid[x];
    dev->type = DEV_BLK;
    dev->subtype = DEV_IDE;
    if (dma)
    {
        dev->bread = (void *)ide_dma_read;
        dev->bwrite = (void *)ide_dma_write;
    }
    else
    {
        dev->bread = (void *)ide_pio_read;
        dev->bwrite = (void *)ide_pio_write;
    }
    dev->mkname = (void *)ide_mkname;
    dev->pdata = pri;
    dev_register(NULL, dev);

    DEBUGK(K_DEV | K_SYNC, "disk : %s\n", dev->name);
    DEBUGK(K_DEV | K_SYNC, "disk sn : %s\n", pri->serial_num);
    DEBUGK(K_DEV | K_SYNC, "disk model : %s\n", pri->model_num);
    DEBUGK(K_DEV | K_SYNC, "iobase = %x, bmbase = %x\n", pri->iobase, pri->bmbase);
}

void ide_init()
{
    u16 bmbase = 0;
    pci_idx_t *idx = pci_find_class(0x0101, 0);
    if (idx != NULL)
    {
        pci_bar_t bar;
        pci_get_bar(idx, &bar, 4);
        bmbase = bar.base;
        pci_set_busmaster(idx);
    }

    init(0, bmbase);
    init(1, bmbase);
    init(2, bmbase);
    init(3, bmbase);

    outb(0x3F6, 0);

#if CONFIG_IDE_USE_INTR
    intr_register(INT_PRIDISK, ide_handler);
    intr_register(INT_SECDISK, ide_handler);
    ioapic_rteset(IRQ_PRIDISK, _IOAPIC_RTE(INT_PRIDISK));
    ioapic_rteset(IRQ_SECDISK, _IOAPIC_RTE(INT_SECDISK));
#endif
}

#include <io.h>
#include <irq.h>
#include <intr.h>
#include <textos/dev.h>
#include <textos/debug.h>
#include <textos/printk.h>

#define SECT_SIZ 512

#include <string.h>

#define IDE_P_BASE 0x1F0 // Prime
#define IDE_S_BASE 0x170 // Secondary

#define R_DATA (0) // 
#define R_ERRF (1) // Error or features
#define R_SECC (2) // Sector count
#define R_LBAL (3) // LBA low
#define R_LBAM (4) // LBA mid
#define R_LBAH (5) // LBA high
#define R_DEV  (6) // Device
#define R_SCMD (7) // Status or cmd

#define R_BCTL 0x206 // Offset of the block register

typedef struct {
    char  serial_num[21];
    char  model_num[41];

    u16   port;
    u8    dev;
} pri_t;

static pri_t info;

#define STAT_ERR   (1 << 0) // Error
#define STAT_IDX   (1 << 2) // Index
#define STAT_CD    (1 << 3) // Corrected data
#define STAT_RQRDY (1 << 4) // Data request ready
#define STAT_WF    (1 << 5) // Drive write fault
#define STAT_RDY   (1 << 6) // Drive ready
#define STAT_BSY   (1 << 7) // Drive Busy

//

#define DEV_PRI (0 << 4)
#define DEV_SEC (1 << 4)

#define DEV_CHS (0 << 6)
#define DEV_LBA (1 << 6)

/* Macro to set the device register, and
   the last binary to set bits which are always `1` */
#define SET_DEV(opt) ((u8)(opt | DEV_LBA | 0b10100000))

#define CMD_IDENT 0xE0
#define CMD_READ  0x20
#define CMD_WRITE 0x30

#include <textos/task.h>

/* TODO: Lock device */

static int pid = -1;

__INTR_HANDLER(ide_handler)
{
    lapic_sendeoi();

    if (pid < 0)
        return;

    DEBUGK(K_DEV, "Read opt has finished, waking proc... -> %d\n", pid);
    task_unblock (pid);
}

static void read_sector (u16 port, u16 *data)
{
    __asm__ volatile (
        "cld\n"
        "rep insw\n" 
        : : "c"(256), "d"(port + R_DATA), "D"(data)
        : "memory"
        );
}

static void write_sector (u16 port, u16 *data)
{
    __asm__ volatile (
        "cld\n"
        "rep outsw\n" 
        : : "c"(256), "d"(port + R_DATA), "D"(data)
        : "memory"
        );
}

void ide_read (dev_t *dev, u32 lba, void *data, u8 cnt)
{
    /*
       我们先 使用 28 位 PIO 模式练练手.
    */
    lba &= 0xFFFFFFF;

    while (inb(R_SCMD) & STAT_BSY) ;

    pri_t *pri = dev->pdata;

    outb(pri->port + R_DEV,  SET_DEV(pri->dev) | DEV_LBA | (lba >> 24)); // 选择设备并发送 LBA 的高4位
    outb(pri->port + R_SECC, cnt);                             // 扇区数目
    outb(pri->port + R_LBAL, lba & 0xFF);                      // LBA 0_7
    outb(pri->port + R_LBAM, (lba >> 8) & 0xFF);               // LBA 8_15
    outb(pri->port + R_LBAH, (lba >> 16) & 0xFF);              // LBA 16_23
    outb(pri->port + R_SCMD, CMD_READ);                        // 读取指令

    pid = task_current()->pid;
    task_block();

    for (int i = 0 ; i < cnt ; i++) {
        read_sector (pri->port, data);
        data += SECT_SIZ;
    }
}

void ide_write (dev_t *dev, u32 lba, void *data, u8 cnt)
{
    UNINTR_AREA_START();

    lba &= 0xFFFFFFF;
    
    pri_t *pri = dev->pdata;

    while (inb(R_SCMD + pri->port) & STAT_BSY) ;

    outb(pri->port + R_DEV,  SET_DEV(pri->dev) | DEV_LBA | (lba >> 24)); // 选择设备并发送 LBA 的高4位
    outb(pri->port + R_SECC, cnt);                                       // 扇区数目
    outb(pri->port + R_LBAL, lba & 0xFF);                                // LBA 0_7
    outb(pri->port + R_LBAM, (lba >> 8) & 0xFF);                         // LBA 8_15
    outb(pri->port + R_LBAH, (lba >> 16) & 0xFF);                        // LBA 16_23
    outb(pri->port + R_SCMD, CMD_WRITE);                                 // 写入指令

    for (int i = 0 ; i < cnt ; i++) {
        write_sector (pri->port, data);
        data += SECT_SIZ;

        pid = task_current()->pid;
        task_block();
    }
    
    UNINTR_AREA_END();
}

/* Details in `/usr/include/linux/hdreg.h` on your pc [doge] */

#define ID_CONFIG   0   // Bit flags                  1  (word)
#define ID_SN       10  // Serial number              10 (word)
#define ID_FWVER    23  // Firmware version           8  (word)
#define ID_MODEL    27  // Model number               20 (word)
#define ID_SUPPORT  49  // Capabality                 1  (byte)
#define ID_ADDR_28  60  // 28位可寻址的全部逻辑扇区数 2  (word)
#define ID_ADDR_48  100 // 28位可寻址的全部逻辑扇区数 3  (word)
#define ID_MSN      176 // Current media sn           60 (word)

#define TRY(count, opts)                            \
    for ( int __try_count__ = count ;               \
              __try_count__ > 0 || count == 0 ;     \
              __try_count__-- )                     \
        opts

static inline void ide_wait ()
{
    while (inb(R_SCMD) & STAT_BSY) ;
}

#include <textos/mm.h>

static bool ide_identify (pri_t **pri, int idx)
{
    pri_t info;
    memset (&info, 0, sizeof(pri_t));

    u16 Buffer[256];

    u8  dev  = idx < 2 ? DEV_PRI : DEV_SEC;
    u16 port = idx & 1 ? IDE_S_BASE : IDE_P_BASE;

    outb(port + R_DEV , SET_DEV(dev));
    outb(port + R_SCMD, 0xEC);

    for (int i = 0 ; i < 0xFFFF ; i++) ;

    u8 stat = inb(port + R_SCMD);
    if (stat == 0 || stat & STAT_ERR) // 没有这个设备, 或者错误
        return false;                 // 中断识别

    read_sector (port, Buffer);
    
    for (int i = 0 ; i < 10 ; i++) {
        info.serial_num[i*2  ] = Buffer[ID_SN + i] >> 8;
        info.serial_num[i*2+1] = Buffer[ID_SN + i] &  0xFF;
    }

    for (int i = 0 ; i < 20 ; i++) {
        info.model_num[i*2  ] = Buffer[ID_MODEL + i] >> 8;
        info.model_num[i*2+1] = Buffer[ID_MODEL + i] &  0xFF;
    }

    info.dev = dev;
    info.port = port;

    DEBUGK (K_DEV | K_SYNC, "disk sn : %s\n", info.serial_num);
    DEBUGK (K_DEV | K_SYNC, "disk model : %s\n", info.model_num);
    DEBUGK (K_DEV | K_SYNC, "disk port base : %#x\n", info.port);
    DEBUGK (K_DEV | K_SYNC, "disk dev : %d\n", info.dev);

    *pri = malloc(sizeof(pri_t));
    memcpy (*pri, &info, sizeof(pri_t));

    return true;
}

/* TODO : Detect all devices */
void ide_init()
{
    outb(0x3F6, 0);

    dev_t *dev;
    pri_t *pri;

    int i = 0;
    for ( ; i < 2 ; i++) {
        if (ide_identify(&pri, i)) {
            dev = dev_new();
            dev->name = pri->model_num;
            dev->type = DEV_BLK;
            dev->subtype = DEV_IDE;
            dev->bread = (void *)ide_read; // 丝毫不费脑筋的,降低代码安全性的强制类型转换...
            dev->bwrite = (void *)ide_write;
            dev->pdata = pri;
            dev_register (dev);
        }
    }

    intr_register (INT_MDISK, ide_handler);
    ioapic_rteset (IRQ_MDISK, _IOAPIC_RTE(INT_MDISK));
    
    for ( ; i < 4 ; i++) {
        if (ide_identify(&pri, i)) {
            dev = dev_new();
            dev->name = pri->model_num;
            dev->type = DEV_BLK;
            dev->subtype = DEV_IDE;
            dev->bread = (void *)ide_read; // 丝毫不费脑筋的,降低代码安全性的强制类型转换...
            dev->bwrite = (void *)ide_write;
            dev->pdata = pri;
            dev_register (dev);
        }
    }
    
    intr_register (INT_SDISK, ide_handler);
    ioapic_rteset (IRQ_SDISK, _IOAPIC_RTE(INT_SDISK));

    DEBUGK(K_INIT | K_SYNC, "disk initialized!\n");
}


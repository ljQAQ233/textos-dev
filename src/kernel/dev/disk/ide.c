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

static u16 port;

#define R_DATA (port + 0) // 
#define R_ERRF (port + 1) // Error or features
#define R_SECC (port + 2) // Sector count
#define R_LBAL (port + 3) // LBA low
#define R_LBAM (port + 4) // LBA mid
#define R_LBAH (port + 5) // LBA high
#define R_DEV  (port + 6) // Device
#define R_SCMD (port + 7) // Status or cmd

#define R_BCTL 0x206 // Offset of the block register

typedef struct {
    char  serial_num[21];
    char  model_num[41];
    dev_t dev;
} info_t;

static info_t info;

#define STAT_DF  (1 << 5)
#define STAT_BSY (1 << 7)

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

static void read_sector (u16 *data)
{
    __asm__ volatile (
        "cld\n"
        "rep insw\n" 
        : : "c"(256), "d"(R_DATA), "D"(data)
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

    outb(R_DEV,  DEV_PRI | DEV_LBA | (lba >> 24)); // 选择设备并发送 LBA 的高4位
    outb(R_SECC, cnt);                             // 扇区数目
    outb(R_LBAL, lba & 0xFF);                      // LBA 0_7
    outb(R_LBAM, (lba >> 8) & 0xFF);               // LBA 8_15
    outb(R_LBAH, (lba >> 16) & 0xFF);              // LBA 16_23
    outb(R_SCMD, CMD_READ);                        // 读取指令

    pid = task_current()->pid;
    task_block();

    for (int i = 0 ; i < cnt ; i++) {
        read_sector (data);
        data += SECT_SIZ;
    }
}

// void IdeRead (u32 Lba, void *Data, u8 Cnt)
// {
// }

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

static void ide_identify ()
{
    info_t Info;
    memset (&Info, 0, sizeof(info_t));

    u16 Buffer[256];

    outb(R_DEV , SET_DEV(DEV_PRI));
    outb(R_SCMD, 0xEC);

    for (int i = 0 ; i < 0xFFFF ; i++) ;

    read_sector (Buffer);
    
    for (int i = 0 ; i < 10 ; i++) {
        Info.serial_num[i*2  ] = Buffer[ID_SN + i] >> 8;
        Info.serial_num[i*2+1] = Buffer[ID_SN + i] &  0xFF;
    }

    for (int i = 0 ; i < 20 ; i++) {
        Info.model_num[i*2  ] = Buffer[ID_MODEL + i] >> 8;
        Info.model_num[i*2+1] = Buffer[ID_MODEL + i] &  0xFF;
    }

    DEBUGK(K_INIT | K_SYNC, "disk sn : %s\n", Info.serial_num);
    DEBUGK(K_INIT | K_SYNC, "disk model : %s\n", Info.model_num);
}

/* TODO : Detect all devices */
void ide_init()
{
    port = IDE_P_BASE;

    ide_identify();

    outb(0x3F6, 0);

    dev_t *disk = dev_new();
    disk->name = "ATA device";
    disk->type = DEV_BLK;
    disk->subtype = DEV_IDE;
    disk->bread = (void *)ide_read; // 丝毫不费脑筋的,降低代码安全性的强制类型转换...
    dev_register (disk);

    intr_register (INT_MDISK, ide_handler);
    ioapic_rteset (IRQ_MDISK, _IOAPIC_RTE(INT_MDISK));
    DEBUGK(K_INIT | K_SYNC, "disk initialized!\n");
}


#include <acpispec/tables.h>
#include <lai/deps.h>
#include <lai/host.h>
#include <textos/mm/map.h>

typedef struct _packed
{
    acpi_header_t hdr;
    u8 hwrevid;
    u8 comparator_cnt : 5;
    u8 counter_size : 1;
    u8 reserved : 1;
    u8 legacy_replacement : 1;
    u16 pci_vendid;
    acpi_address_t address;
    u8 hpetnr;
    u16 mintick;
} acpi_hpet_t;

// HPET能力/ID寄存器结构
typedef union {
    u64 raw;
    struct
    {
        u32 rev_id : 8;        // 修订版本号
        u32 num_timers : 5;    // 定时器数量-1
        u32 counter_size : 1;  // 计数器大小(1=64位, 0=32位)
        u32 reserved0 : 1;     // 保留
        u32 legacy_route : 1;  // 
        u32 vendid : 16;       // 厂商ID
        u32 period : 32;       // 计数器时钟周期(飞秒)
    } _packed;
} hpet_capid;

// HPET配置寄存器结构
typedef union {
    u64 raw;
    struct
    {
        u32 enable : 1;       // 主计数器使能
        u32 legacy_route : 1; // 传统替换路由使能
        u32 reserved0 : 30;   // 保留
        u32 reserved1 : 32;   // 保留
    } _packed;
} hpet_config;

// 定时器配置/能力寄存器结构
typedef union {
    u64 raw;
    struct
    {
        u32 reserved0 : 1;        // 保留
        u32 level_triggered : 1;  // 中断触发模式(1=电平触发)
        u32 interrupt_enable : 1; // 中断使能
        u32 periodic : 1;         // 周期模式使能
        u32 periodic_cap : 1;     // 周期模式能力
        u32 size_cap : 1;         // 64位计数器能力
        u32 set_val : 1;          // 累加模式设置
        u32 reserved1 : 1;        // 保留
        u32 int_route : 5;        // 中断路由
        u32 fsb_enable : 1;       // FSB传输使能
        u32 fsb_int_del_cap : 1;  // FSB中断传输能力
        u32 reserved2 : 16;       // 保留
        u32 int_route_cap : 32;   // 中断路由能力
    } _packed;
} hpet_timer_config;

// HPET寄存器空间完整结构
typedef struct
{
    hpet_capid capid;   // 0x000
    u64 reserved0;      // 0x008
    hpet_config config; // 0x010
    u64 reserved1;      // 0x018
    u64 intr_stat;      // 0x020
    u64 reserved2[25];  // 0x028-0x0DF
    u64 main_counter;   // 0x0F0
    u64 reserved3;      // 0x0F8-0x0FF

    // 定时器寄存器块 (每个定时器4个寄存器)
    struct
    {
        hpet_timer_config config; // 配置寄存器
        u64 comparator;           // 比较器寄存器
        u64 fsb_route;            // FSB中断路由寄存器
        u64 reserved;             // 保留
    } timer[32];                  // 最多支持32个定时器
} hpet_t;

#define HPET_WR(ptr, field, val) do {          \
    __typeof__(*(ptr)) __tmp = *(ptr);         \
    __tmp.field = (val);                       \
    (* (volatile u64 *)((addr_t)ptr & ~7ul)) = \
        *(u64 *)((addr_t)&__tmp);              \
} while (0)

#define HPET_RD(ptr, field, val) do {          \
    __typeof__(*(ptr)) __tmp = *(ptr);         \
    val = __tmp.field;                         \
} while (0)

static hpet_t *hpet;
static acpi_hpet_t *table;

void hpet_init()
{
    table = laihost_scan("HPET", 0);
    vmap_map(table->address.addr, __acpi_pages,
        DIV_ROUND_UP(sizeof(hpet_t), PAGE_SIZ),
        PE_P | PE_RW | PE_PCD | PE_PWT, MAP_4K);
    hpet = (hpet_t *)__acpi_pages;

    HPET_WR(&hpet->config, enable, 1);
}

u64 hpet_get_tick()
{
    u64 tick;
    HPET_RD(hpet, main_counter, tick);
    return tick;
}

/*
 * hpet period is stored as a type of uint32
 * one tick means `period` fs (femptosecond)
 */
u64 hpet_get_period()
{
    u64 period;
    HPET_RD(&hpet->capid, period, period);
    return period;
}

u64 hpet_get_fs()
{
    u64 tick = hpet_get_tick();
    u64 period_fs = hpet_get_period();
    return tick * period_fs;
}

u64 hpet_get_us()
{
    return hpet_get_fs() / 1000000000ull;
}

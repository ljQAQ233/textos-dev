#include <cpu.h>
#include <gdt.h>
#include <string.h>
#include <textos/assert.h>

typedef struct _packed
{
    u16 limit; // gdt占用
    u64 base;  // gdt的地址
} gdtr_t;

typedef struct _packed
{
    u16 limit_low : 16;
    u32 base_low : 24;
    union
    {
        struct
        {
            u8 A : 1;
            u8 RW : 1;
            u8 DC : 1;
            u8 EXE : 1;
            u8 S : 1;
            u8 DPL : 2;
            u8 P : 1;
        };
        u8 raw : 8;
    } access; // access byte
    u8 limit_high : 4;
    u8 flgs : 4;
    u8 base_high : 8;
} gdt_t;

typedef struct _packed
{
    gdt_t cap; // 向下兼容的哪一部分
    u32 ext;   // 拓展的一部分
    u32 rev;   // 保留
} sys_t;

STATIC_ASSERT(sizeof(gdt_t) == 8, "");
STATIC_ASSERT(sizeof(sys_t) == 16, "");

typedef struct _packed
{
    u32 rev0;
    u64 rsp0;
    u64 rsp1;
    u64 rsp2;
    u32 rev1;
    u32 rev2;
    u64 ist1;
    u64 ist2;
    u64 ist3;
    u64 ist4;
    u64 ist5;
    u64 ist6;
    u64 ist7;
    u32 rev3;
    u32 rev4;
    u16 rev5;
    u16 iopb;
} tss_t;

#define GDT_MAX 16 // 16 个绝对够你用啦! /doge

#define F_L  (1 << 1) // Long mode
#define F_DB (1 << 2)
#define F_G  (1 << 3)

#define A_A        (1)
#define A_RW       (1 << 1)
#define A_DC       (1 << 2)
#define A_EXE      (1 << 3)
#define A_NS       (1 << 4) // isn't system segment
#define A_P        (1 << 7)
#define A_DPL(DPL) ((DPL & 0x3) << 5)

#define A_AVA (0x9) // tss available

gdt_t __gdts[GDT_MAX];
gdtr_t __gdtr;

static void _gdt_set_entry(size_t i, u64 base, u32 limit, u8 access, u8 flgs)
{
    gdt_t* entry = &__gdts[i];
    entry->base_low = base & 0xffffff;
    entry->base_high = base >> 24 & 0xff;
    entry->limit_low = limit & 0xffff;
    entry->limit_high = limit >> 16 & 0xff;
    entry->flgs = flgs;
    entry->access.raw = access;
}

static void _sys_set_entry(size_t i, u64 base, u32 limit, u8 access, u8 flgs)
{
    sys_t* ptr = (sys_t*)&__gdts[i];
    ptr->cap.base_low = base & 0xffffff;
    ptr->cap.base_high = base >> 24 & 0xff;
    ptr->cap.limit_low = limit & 0xffff;
    ptr->cap.limit_high = limit >> 16 & 0xff;
    ptr->cap.flgs = flgs;
    ptr->cap.access.raw = access;
    ptr->ext = base >> 32;
    ptr->rev = 0;
}

#define KERN_CODE_A (A_P | A_DPL(0) | A_RW | A_EXE | A_NS)
#define KERN_DATA_A (A_P | A_DPL(0) | A_RW | A_NS)

#define USER_CODE_A (A_P | A_DPL(3) | A_RW | A_EXE | A_NS)
#define USER_DATA_A (A_P | A_DPL(3) | A_RW | A_NS)

#define CODE_F (F_L | F_G)
#define DATA_F (F_DB | F_G)

tss_t tss;

void gdt_init()
{
    memset(__gdts, 0, GDT_MAX * sizeof(gdt_t));

    /* Keep the firt being 0 */

    _gdt_set_entry(KERN_CODE_SEG, 0, 0xFFFFF, KERN_CODE_A, CODE_F);
    _gdt_set_entry(KERN_DATA_SEG, 0, 0xFFFFF, KERN_DATA_A, DATA_F);
    _gdt_set_entry(USER_CODE_SEG, 0, 0xFFFFF, USER_CODE_A, CODE_F);
    _gdt_set_entry(USER_DATA_SEG, 0, 0xFFFFF, USER_DATA_A, DATA_F);
    _gdt_set_entry(USER_CODE32_SEG, 0, 0xFFFFF, USER_DATA_A, CODE_F & ~F_L);
    _sys_set_entry(TSS_LOAD_SEG, (u64)&tss, sizeof(tss) - 1, A_AVA | A_P, 0);

    __gdtr.base = (u64)&__gdts;
    __gdtr.limit = GDT_MAX * sizeof(gdt_t);

    load_gdt(&__gdtr);
    reload_segs(KERN_DATA_SEG << 3, KERN_CODE_SEG << 3);

    load_tss(TSS_LOAD_SEG << 3);
}

void __tss_set(u64 rsp)
{
    tss.rsp0 = rsp;
}

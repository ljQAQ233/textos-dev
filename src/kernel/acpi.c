/*
 * 3 phases to setup acpi
 *   - prepare - get physical address from boot config
 *   - acpi_init - find out some essential info
 *   - tovmm - map all table to kernel space and enable lai
 */

#define RSDP_SIG SIGN_64('R', 'S', 'D', ' ', 'P', 'T', 'R', ' ')

/* Root System Description Pointer */
typedef struct _packed
{
    u64 sign;
    u8 chksum;
    char oem_id[6];
    u8 revision;
    u32 rsdt_addr;
    u32 len;
    u64 xsdt_addr;
    u8 ext_chksum;
    u8 rev[3];
} rsdp_t;

#define RSDT_SIG SIGN_32('R', 'S', 'D', 'T')
#define XSDT_SIG SIGN_32('X', 'S', 'D', 'T')
#define MADT_SIG SIGN_32('A', 'P', 'I', 'C')
#define FADT_SIG SIGN_32('F', 'A', 'C', 'P')

typedef struct _packed
{
    u32 sign;
    u32 len;
    u8 revision;
    u8 chksum;
    char oem_id[6];
    char oem_tabid[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
} hdr_t;

typedef struct _packed
{
    hdr_t hdr;
} rsdt_t;

/* eXtended System Description Table */
typedef struct _packed
{
    hdr_t hdr;
} xsdt_t;

#include <textos/panic.h>
#include <textos/printk.h>

#include <textos/boot.h>
#include <textos/klib/string.h>

rsdp_t *__rsdp;
rsdt_t *__rsdt;
xsdt_t *__xsdt;

static bool chksum(void *buf, size_t siz)
{
    u8 s = 0;
    for (size_t i = 0; i < siz; i++)
        s += ((char *)buf)[i];

    return (s == 0);
}

static void *entryget(u32 sign)
{
    hdr_t *tbl = (void *)__xsdt ?: (void *)__rsdt;
    if (!tbl)
        return NULL;

    union {
        u32 *u32;
        u64 *u64;
    } adrs = {.u32 = (void *)tbl + sizeof(hdr_t)};
    size_t entsz = __xsdt ? 8 : 4;
    size_t n = (tbl->len - sizeof(hdr_t)) / entsz;
    for (size_t i = 0; i < n; i++)
    {
        addr_t addr = entsz == 8 ? adrs.u64[i] : adrs.u32[i];
        hdr_t *hdr = (hdr_t *)addr;
        if (hdr->sign == sign && chksum(hdr, hdr->len))
            return hdr;
    }
    return NULL;
}

#include <textos/mm.h>
#include <textos/mm/pvpage.h>

static void madt_parser();

void acpi_init()
{
    rsdp_t *rsdp = __rsdp;

    /* This includes only the first 20 bytes of this Rsdp,
       including the checksum field. These bytes must sum
       to zero.
       NOTE : Overflow to zero. (char/unsigned char/u8)   */
    if (rsdp->sign != RSDP_SIG || !chksum(rsdp, 20))
        PANIC("bad RSDP!!!\n");

    rsdt_t *rsdt = (rsdt_t *)(addr_t)rsdp->rsdt_addr;
    if (rsdt->hdr.sign != RSDT_SIG || !chksum(rsdt, rsdt->hdr.len))
        PANIC("bad Rsdt!!!");

    xsdt_t *xsdt = (xsdt_t *)rsdp->xsdt_addr;
    if (xsdt != NULL)
    {
        if (xsdt->hdr.sign != XSDT_SIG || !chksum(xsdt, xsdt->hdr.len))
            PANIC("bad Xsdt!!!");
    }

    __rsdt = rsdt;
    __xsdt = xsdt;

    madt_parser();
}

void __acpi_pre()
{
    if (bmode_get() == BOOT_EFI)
    {
        bconfig_t *b = binfo_get();
        __rsdp = b->acpi;
    }
    if (bmode_get() == BOOT_MB1)
    {
        addr_t scan = 0xe0000;
        addr_t scan_end = 0x100000;
        for (; scan < scan_end; scan += 16)
        {
            u64 *sig = (u64 *)scan;
            if (*sig == RSDP_SIG)
                break;
        }
        __rsdp = (void *)scan;
    }
}

#include <lai/helpers/sci.h>
#include <textos/mm.h>
#include <textos/mm/pvpage.h>

#define align_up(x, y) ((y) * ((x + y - 1) / y))
#define align_down(x, y) ((y) * (x / y))

addr_t acpi_map(addr_t pa, size_t num)
{
    static size_t idx = 0;
    addr_t va = __acpi_pages + PAGE_SIZ * idx;
    vmap_map(pa, va, num, PE_P | PE_RW);
    idx += num;
    return va;
}

void *acpi_remap(void *ptr)
{
    addr_t addr;
    addr = (addr_t)ptr;
    addr = align_down(addr, PAGE_SIZ);

    size_t len;
    hdr_t *hp = ptr;
    rsdp_t *rp = ptr;
    if (rp->sign == RSDP_SIG)
    {
        len = rp->len;
        if (rp->revision <= 1)
            len = sizeof(rsdp_t);
    }
    else
        len = hp->len;

    addr_t end;
    end = addr + len;
    end = align_up(end, PAGE_SIZ);

    size_t pgnr = (end - addr) / PAGE_SIZ;
    addr_t pgva = acpi_map(addr, pgnr);
    addr_t low = (addr_t)ptr & PAGE_MASK;
    return (void *)(pgva | low);
}

#include <textos/klib/list.h>

static list_t kacpi;

typedef struct
{
    u32 sign;
    addr_t pa;
    addr_t va;
    list_t list;
} acpi_tab_t;

// register a table and record its vrt addr
void *acpi_regtab(hdr_t *hdr)
{
    void *vpg = acpi_remap((void *)hdr);
    acpi_tab_t *t = malloc(sizeof(*t));
    t->sign = hdr->sign;
    t->pa = (addr_t)hdr;
    t->va = (addr_t)vpg;
    list_insert(&kacpi, &t->list);

    char *sig = (char *)(&t->sign);
    DEBUGK(K_INIT, "ACPI REG %c%c%c%c\n", sig[0], sig[1], sig[2], sig[3]);
    return vpg;
}

void *acpi_gettab(u32 sign, int idx)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &kacpi)
    {
        acpi_tab_t *t = CR(ptr, acpi_tab_t, list);
        if (t->sign == sign && idx-- == 0)
            return (void *)t->va;
    }
    return NULL;
}

/*
 * remap and lai setup
 */
void __acpi_tovmm()
{
    list_init(&kacpi);

    // top tables
    __rsdp = acpi_remap(__rsdp);
    __rsdt = acpi_regtab(&__rsdt->hdr);
    if (__xsdt)
        __xsdt = acpi_regtab(&__xsdt->hdr);

    // table and extened table
    u32 *rtab = (void *)__rsdt + sizeof(rsdt_t);
    for (int i = 0; i < (__rsdt->hdr.len - sizeof(rsdt_t)) / 4; i++)
        acpi_regtab((hdr_t *)(addr_t)rtab[i]);

    if (__xsdt)
    {
        u64 *xtab = (void *)__xsdt + sizeof(xsdt_t);
        for (int i = 0; i < (__xsdt->hdr.len - sizeof(xsdt_t)) / 8; i++)
            acpi_regtab((hdr_t *)(addr_t)xtab[i]);
    }

    // parse subtable - DSDT
    acpi_fadt_t *fadt = acpi_gettab(FADT_SIG, 0);
    acpi_regtab((hdr_t *)(addr_t)(__rsdp->revision <= 1 ? fadt->dsdt : fadt->x_dsdt));

    // lai init
    // ioapic mode
    lai_create_namespace();
    lai_enable_acpi(1);
}

#define APIC_SIG SIGN_32('A', 'P', 'I', 'C')

/* Multiple APIC Description Table */
typedef struct _packed
{
    hdr_t hdr;
    u32 lapic_addr;
    u32 flgs;
} madt_t;

typedef struct _packed
{
    u8 type;
    u8 len;
} madt_ics_t;

typedef struct _packed
{
    madt_ics_t hdr;
    u8 apic_uid; // Apic Processor UID
    u8 apic_id;
    u32 flgs;
} ics_lapic_t;

enum ics_type
{
    ICS_LAPIC = 0,
    ICS_IOAPIC = 1,
    ICS_ISO = 2,
};

typedef struct _packed
{
    madt_ics_t hdr;
    u8 ioapic_id;
    u8 rev;
    u32 ioapic_addr;
    u32 gsi_base;
} ics_ioapic_t;

typedef struct _packed
{
    madt_ics_t hdr;
    u8 bus;
    u8 src;
    u32 gsi;
    u16 flgs;
} ics_iso_t;

extern void *lapic;
extern void *ioapic;

static u8 _gsi[24];

/* Src 到 Gsi 的过程类似于一个重定向的过程 */
u8 __gsiget(u8 src)
{
    return _gsi[src] != 0xFF ? _gsi[src] : src;
}

static void madt_parser()
{
    madt_t *madt = entryget(APIC_SIG);
    if (madt == NULL)
        PANIC("can not find madt!!!");

    int64 len = madt->hdr.len - sizeof(madt_t);
    madt_ics_t *ics = OFFSET(madt, sizeof(madt_t));

    memset(_gsi, 0xFF, 24);
    while (len > 0)
    {
        switch (ics->type)
        {
        case ICS_LAPIC:
            // 写 `{}` 是明确变量的 life circle
            {
                ics_lapic_t *_lapic = (ics_lapic_t *)ics;
                printk("local Apic -> acpi processor uid : %u\n"
                       "              apic id : %u\n"
                       "              flags :   %x\n",
                       _lapic->apic_uid, _lapic->apic_id, _lapic->flgs);
            }
            break;

        case ICS_IOAPIC: {
            ics_ioapic_t *_ioapic = (ics_ioapic_t *)ics;
            printk("i/o apic -> i/o apic id : %u\n"
                   "            i/o apic addr : %#x\n"
                   "            global system interrupt base : %#x\n",
                   _ioapic->ioapic_id, _ioapic->ioapic_addr, _ioapic->gsi_base);
            ioapic = (void *)(u64)_ioapic->ioapic_addr;
        }
        break;

        /* Interrupt Source Override */
        case ICS_ISO: {
            ics_iso_t *_iso = (ics_iso_t *)ics;
            printk("intr src override -> bus : %u, src : %u, flgs : %x\n"
                   "                     global sys intr : %u\n",
                   _iso->bus, _iso->src, _iso->flgs, _iso->gsi);
        }
        break;
        }
        len -= ics->len;
        ics = OFFSET(ics, ics->len);
    }

    lapic = (void *)(u64)madt->lapic_addr;
}

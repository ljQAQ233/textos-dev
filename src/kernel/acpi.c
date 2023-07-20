#define RSDP_SIG SIGN_64('R','S','D',' ','P','T','R',' ')

/* Root System Description Pointer */
typedef struct _packed {
    u64  sign;
    u8   chksum;
    char oem_id[6];
    u8   revision;
    u32  rsdt_addr;
    u32  len;
    u64  xsdt_addr;
    u8   ext_chksum;
    u8   rev[3];
} rsdp_t;

#define XSDT_SIG SIGN_32('X','S','D','T')
#define MADT_SIG SIGN_32('A','P','I','C')

typedef struct _packed {
    u32  sign;
    u32  len;
    u8   revision;
    u8   chksum;
    char oem_id[6];
    char oem_tabid[8];
    u32  oem_revision;
    u32  creator_id;
    u32  creator_revision;
} hdr_t;

/* eXtended System Description Table */
typedef struct _packed {
    hdr_t hdr;
} xsdt_t;

#include <textos/panic.h>
#include <textos/printk.h>

#include <boot.h>
#include <string.h>

xsdt_t *__xsdt;

static bool _chksum (void * buf, size_t siz)
{
    u8 s = 0;
    for (size_t i = 0; i < siz; i++)
        s += ((char *)buf)[i];

    return (s == 0);
}

static void *_entryget (u32 sign)
{
    size_t num = (__xsdt->hdr.len - sizeof(xsdt_t)) / 8;

    hdr_t *ptr;
    u64 *addrs = (void *)__xsdt + sizeof(xsdt_t);

    for (size_t i = 0; i < num ;i++) {
        ptr = (hdr_t *)(addrs[i]);
        if (ptr->sign == sign) {
            if (_chksum(ptr, ptr->len))
                return ptr;
        }
    }
    return NULL;
}

void madt_parser ();
void fadt_parser ();

static rsdp_t *rsdp_phy;

void acpi_init ()
{
    rsdp_t *rsdp = rsdp_phy;

    /* This includes only the first 20 bytes of this Rsdp,
       including the checksum field. These bytes must sum
       to zero. 
       NOTE : Overflow to zero. (char/unsigned char/u8)   */
    if (rsdp->sign != RSDP_SIG || !_chksum(rsdp, 20))
        PANIC ("bad RSDP!!!\n");

    xsdt_t *xsdt = (xsdt_t *)rsdp->xsdt_addr;
    if (xsdt->hdr.sign != XSDT_SIG || !_chksum(xsdt, xsdt->hdr.len))
        PANIC ("bad Xsdt!!!");
    __xsdt = xsdt;

    madt_parser();
    fadt_parser();
}

void __acpi_pre (void *acpi)
{
    rsdp_phy = acpi;
}

#define APIC_SIG SIGN_32('A','P','I','C')

/* Multiple APIC Description Table */
typedef struct _packed {
    hdr_t hdr;
    u32   lapic_addr;
    u32   flgs;
} madt_t;

typedef struct _packed {
    u8 type;
    u8 len;
} madt_ics_t;

typedef struct _packed {
    madt_ics_t hdr;
    u8        apic_uid; // Apic Processor UID
    u8        apic_id;
    u32       flgs;
} ics_lapic_t;

enum ics_type
{
    ICS_LAPIC  = 0,
    ICS_IOAPIC = 1,
    ICS_ISO    = 2,
};

typedef struct _packed {
    madt_ics_t hdr;
    u8         ioapic_id;
    u8         rev;
    u32        ioapic_addr;
    u32        gsi_base;
} ics_ioapic_t;

typedef struct _packed {
    madt_ics_t hdr;
    u8         bus;
    u8         src;
    u32        gsi;
    u16        flgs;
} ics_iso_t;

extern void *lapic;
extern void *ioapic;

void madt_parser ()
{
    madt_t *madt = _entryget (APIC_SIG);
    if (madt == NULL)
        PANIC ("can not find madt!!!");
    
    int64 len = madt->hdr.len - sizeof(madt_t);
    madt_ics_t *ics = OFFSET (madt, sizeof(madt_t));

    while (len > 0) {
        switch (ics->type)
        {
        case ICS_LAPIC:
            // 写 `{}` 是明确变量的 life circle
            {
                ics_lapic_t *_lapic = (ics_lapic_t *)ics;
                printk ("local Apic -> acpi processor uid : %u\n"
                        "              apic id : %u\n"
                        "              flags :   %x\n",
                        _lapic->apic_uid,_lapic->apic_id,_lapic->flgs);
            }
        break;
        
        case ICS_IOAPIC:
            {
                ics_ioapic_t *_ioapic = (ics_ioapic_t *)ics;
                printk ("i/o apic -> i/o apic id : %u\n"
                        "            i/o apic addr : %#x\n"
                        "            global system interrupt base : %#x\n",
                        _ioapic->ioapic_id,_ioapic->ioapic_addr,_ioapic->gsi_base);
                ioapic = (void *)(u64)_ioapic->ioapic_addr;
            }
        break;

        /* Interrupt Source Override */
        case ICS_ISO:
            {
                ics_iso_t *_iso = (ics_iso_t *)ics;
                printk ("intr src override -> bus : %u, src : %u, flgs : %x\n"
                        "                     global sys intr : %u\n",
                        _iso->bus,_iso->src,_iso->flgs,_iso->gsi);
            }
        break;
        }
        len -= ics->len;
        ics = OFFSET (ics, ics->len);
    }

    lapic = (void *)(u64)madt->lapic_addr;
}

typedef struct {
    hdr_t hdr;
} fadt_t;

#define FADT_SIG SIGN_32('F','A','C','P')

void fadt_parser ()
{
    fadt_t *fadt = _entryget (FADT_SIG);
}


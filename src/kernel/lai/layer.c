// os layer

#include <lai/deps.h>
#include <textos/panic.h>

/* Logs a message. level can either be LAI_DEBUG_LOG for debugging info,
   or LAI_WARN_LOG for warnings */
void laihost_log(int level, const char *msg)
{

}

/* Reports a fatal error, and halts. */
__attribute__((noreturn))
void laihost_panic(const char *msg)
{
    dprintk(K_SYNC, "panic!!! - %s", msg);
    __asm__ volatile(
        "cli\n"
        "hlt\n"
    );

    // unreachable
    while (true);
}

#include <io.h>
#include <textos/mm.h>
#include <textos/mm/vmm.h>

/* Self-explanatory */
void *laihost_malloc(size_t size)
{
    return malloc(size);
}

void *laihost_realloc(void *oldptr, size_t newsize, size_t oldsize)
{
    return realloc(oldptr, newsize);
}

void laihost_free(void *ptr, size_t size)
{
    return free(ptr);
}

extern addr_t acpi_map(addr_t pa, size_t num);
extern void *acpi_gettab(u32 sign, int idx);

/* Maps count bytes from the given physical address and returns
   a virtual address that can be used to access the memory. */
void *laihost_map(size_t address, size_t count)
{
    return (void *)acpi_map(address, count);
}

/* Unmaps count bytes from the given virtual address.
   LAI only calls this on memory that was previously mapped by laihost_map(). */
void laihost_unmap(void *pointer, size_t count)
{
    return ;
}

/* Returns the (virtual) address of the n-th table that has the given signature,
   or NULL when no such table was found. */
void *laihost_scan(char *sig, size_t index)
{
    u32 sig32 = *(u32 *)sig;
    return acpi_gettab(sig32, index);
}

/* Write a byte/word/dword to the given I/O port. */
void laihost_outb(uint16_t port, uint8_t val)
{
    return outb(port, val);
}

void laihost_outw(uint16_t port, uint16_t val)
{
    return outw(port, val);
}

void laihost_outd(uint16_t port, uint32_t val)
{
    return outdw(port, val);
}

/* Read a byte/word/dword from the given I/O port. */
uint8_t laihost_inb(uint16_t port)
{
    return inb(port);
}

uint16_t laihost_inw(uint16_t port)
{
    return inw(port);
}

uint32_t laihost_ind(uint16_t port)
{
    return indw(port);
}

#include <textos/dev/pci.h>

/* Write a byte/word/dword to the given device's PCI configuration space
   at the given offset. */
void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val)
{
    pci_write_byte(bus, slot, fun, offset, val);
}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val)
{
    pci_write_word(bus, slot, fun, offset, val);
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val)
{
    pci_write_dword(bus, slot, fun, offset, val);
}

/* Read a byte/word/dword from the given device's PCI configuration space
   at the given offset. */
uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{
    return pci_read_byte(bus, slot, fun, offset);
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{
    return pci_read_word(bus, slot, fun, offset);
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset)
{
    return pci_read_dword(bus, slot, fun, offset);
}

/* Sleeps for the given amount of milliseconds. */
void laihost_sleep(uint64_t ms)
{

}


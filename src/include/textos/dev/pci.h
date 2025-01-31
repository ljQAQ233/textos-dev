#pragma once

#include <textos/klib/list.h>

typedef struct
{
    u16 vendor;
    u16 devid;
    u8 bus;
    u8 slot;
    u8 func;
    list_t all;
} pci_idx_t;

typedef struct
{
    int nr;
    bool mmio;
    u32 base;
    u32 size;
} pci_bar_t;

pci_idx_t *pci_find(u16 vendor, u16 devid, int x);

void pci_get_bar(pci_idx_t *idx, pci_bar_t *barx, int x);

u8 pci_get_intr(pci_idx_t *idx);

void pci_set_busmaster(pci_idx_t *idx);

bool pci_has_caplist(pci_idx_t *idx);

int pci_set_msi(pci_idx_t *idx, u8 vector);
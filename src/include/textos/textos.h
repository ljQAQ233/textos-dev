#ifndef __TEXTOS_H__
#define __TEXTOS_H__

#define __TEXT_OS__

#include <textos/type.h>
#include <textos/base.h>

#include <textos/debug.h>

#define __kern_fb_base   0xffff8a0000000000ULL
#define __kern_fb_maxsz  0x0000010000000000ULL

#define __kern_heap_base 0xffff810000000000ULL
#define __kern_stack_max 0xffff81c000000000ULL // unused -> TODO
#define __kern_stack_top 0xffff820000000000ULL // unused
#define __kern_phy_offet 0xffff830000000000ULL
#define __kern_phy_mapsz 0x0000010000000000ULL

#define __kern_vmm_pages 0xffffb00000000000ull
#define __acpi_pages     0xffffbff000000000ull // acpi mapped

#define __kern_pvpg_base 0xffffc00000000000ull
#define __kern_pvpg_max  0xffffc00000100000ull

#define __uefi_misc      0xffff8c0000000000ull // 64 MiB
#define __uefi_max       0xffff8c0004000000ull

#define __lapic_va  0xffffff0000008000ull
#define __ioapic_va 0xffffff0000000000ull

#define __e1000_va  0xffffff0000010000ull

#define __user_stack_top    0x00007ffffffff000ull
#define __user_stack_bot    0x00007fffffff0000ull
#define __user_stack_pages  ((__user_stack_top - __user_stack_bot) / PAGE_SIZ)

#define istk_pages 1

#endif

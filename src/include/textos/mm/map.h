#pragma once

// Common flags (used in PML4E / PDPTE / PDE / PTE)
#define PE_P      1          // Present; page is present in memory
#define PE_RW    (1 << 1)    // Read/Write; if 0, read-only
#define PE_US    (1 << 2)    // User/Supervisor; if 0, user-mode cannot access
#define PE_PWT   (1 << 3)    // Page-Level Write-Through
#define PE_PCD   (1 << 4)    // Page-Level Cache Disable
#define PE_A     (1 << 5)    // Accessed; set by CPU when page is accessed
#define PE_D     (1 << 6)    // Dirty; set by CPU when page is written
#define PE_NX    (1ULL << 63) // No Execute; if set, instruction fetch is not allowed

// specific flags
#define PDPTE_1G (1 << 7)    // 1 GiB page
#define PDE_2M   (1 << 7)    // 2 MiB page
#define PTE_PAT  (1 << 7)    // Page Attribute Table; controls cache policy
#define PTE_G    (1 << 8)    // Global; page is not flushed from TLB on CR3 reload

/*
 * Notes
 * - PE_P, PE_RW, PE_US are valid in all levels.
 * - PE_A, PE_D are set automatically by the CPU on access/write.
 * - PE_NX requires NX-bit capable CPU, marks non-executable pages.
 * - PDE_2M / PDPTE_1G are used for large page mappings.
 * - PTE_PAT and PTE_G are only meaningful for 4KiB PTEs.
 * 
 * kernel haven't supported hugepage yet.
 */

void vmap_map(addr_t phy, addr_t vrt, size_t num, int flgs);
addr_t vmap_query(addr_t vrt);

addr_t get_kpgt();
addr_t get_kppgt();
addr_t new_pgt();
addr_t copy_pgtd(addr_t ppgt);
void clear_pgt(addr_t ppgt);

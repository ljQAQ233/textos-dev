#pragma once

#define PE_P      1         // Present
#define PE_RW    (1 << 1)   // Read/Write
#define PE_US    (1 << 2)   // User/supervisor; if 0, user-mode accesses are not allowed to the 4K page referenced by this entry
#define PE_PWT   (1 << 3)
#define PE_PCD   (1 << 4)
#define PE_A     (1 << 5)   // This page is accessed by a programe

#define PE_D     (1 << 6)   // Dirty data,all entries can use except for PML4E

#define PDPTE_1G (1 << 7)   // Set a PDPTE to be 1GiB

#define PDE_2M   (1 << 7)   // Set a PDE to be 2MiB

#define PTE_PAT  (1 << 7)
#define PTE_G    (1 << 8)   // Global

enum map_mode {
    MAP_4K = 1, // PTE
    MAP_2M = 2, // PDE
    MAP_1G = 3, // PDPTE
    MAP_ED = 4, // End
};

void vmap_map (u64 phy, u64 vrt, size_t num, u16 flgs, int mode);


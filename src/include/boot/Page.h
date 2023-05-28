#ifndef __PAGE_H__
#define __PAGE_H__

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

/* Mark the mapping of kernel in KernelLoad process */
typedef struct {
  UINT8                Valid;
  UINT32               Flgs;
  UINT64               MemSiz;
  PHYSICAL_ADDRESS     PhyAddr;
  PHYSICAL_ADDRESS     VirtAddr;
} KERNEL_PAGE;

EFI_STATUS
InitializePageTab (
  IN     KERNEL_PAGE *Pages,
     OUT UINT64      *PML4Addr
  );

enum MAP_MODE {
  MAP_4K = 1,
  MAP_2M = 2,
  MAP_1G = 3,
  MAP_ED = 4,
};

EFI_STATUS
Map (
  IN UINT64 PML4Addr,
  IN UINT64 PhyAddr,
  IN UINT64 VirtAddr,
  IN UINT64 Siz,
  IN UINT64 Flgs,
  IN UINT8  Mode
  );

BOOLEAN
IsPageMapped (
  IN UINT64 PML4Addr,
  IN UINT64 VirtAddr
  );

UINT64
VirtToPhyAddr (
  IN UINT64 PML4Addr,
  IN UINT64 VirtAddr
  );

VOID
PML4_Dump (
  IN UINT64 *PML4
  );

/* Update CR3 Register */
UINTN
UpdateCr3 (
  IN UINTN  PML4Addr,
  IN UINT16 FlgsMask
  );

#endif

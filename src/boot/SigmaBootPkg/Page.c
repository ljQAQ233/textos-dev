#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Library/UefiLib.h>

#include <Boot.h>
#include <Page.h>

/* Value */
#define PE_V_FLAGS(Entry) (((UINT64)Entry) & 0x7FF)
#define PE_V_ADDR(Entry)  (((UINT64)Entry) & ~0x7FF)

/* Set */
#define PE_S_FLAGS(Flgs) (UINT64)(Flgs & 0x7FF)
#define PE_S_ADDR(Addr)  (UINT64)(((UINT64)Addr & ~0x7FF))

/* Locate entry or others by virtual address */
#define VIRT_PML4E_IDX(Addr) (((UINT64)Addr >> 39) & 0x1FF)
#define VIRT_PDPTE_IDX(Addr) (((UINT64)Addr >> 30) & 0x1FF)
#define VIRT_PDE_IDX(Addr)   (((UINT64)Addr >> 21) & 0x1FF)
#define VIRT_PTE_IDX(Addr)   (((UINT64)Addr >> 12) & 0x1FF)

/* 页内偏移 */
#define VIRT_OFFSET(Addr)   (((UINT64)Addr & 0xFFF))

/* 根据 PageSiz 来转换单位(大小->页) */
#define SIZE_TO_PAGES(Siz,PageSiz) \
    ((Siz % PageSiz == 0) ? (Siz / PageSiz) : ((Siz - Siz % PageSiz) / PageSiz + 1)) 

#define PAGE_SIZE (0x1000ULL)
#define PT_SIZE   (PAGE_SIZE * 512)
#define PD_SIZE   (PT_SIZE   * 512)
#define PDPT_SIZE (PD_SIZE   * 512)
#define PML4_SIZE (PDPT_SIZE * 512)

STATIC VOID *PageTables;

#define CR0_WP  BIT16

/* 
   Set protection for page tables in Cr0
   
   @param  Stat  TRUE  -> Enable protection
                 FALSE -> Disable protection
*/
VOID PageTableProtect(BOOLEAN Stat)
{
    if (Stat)
    {
        AsmWriteCr0 (AsmReadCr0() | CR0_WP);
    }
    else
    {
        AsmWriteCr0 (AsmReadCr0() & ~CR0_WP);
    }
}

EFI_STATUS InitializePageTab (
        IN     KERNEL_PAGE *Pages,
           OUT UINT64      *PML4Addr
        )
{
    PageTables =  (UINT64 *)AllocatePages (1);
    ZeroMem (PageTables, SIZE_4KB);
    // PageTables =  (UINT64 *)(AsmReadCr3() &~ 0x7ff);

    /* Use the page table which uefi set before */
    UINT64 *PML4 = PageTables;

    /* Set maps for kernel */

    KERNEL_PAGE *Page;
    for (Page = Pages;Page->Valid;Page++)
    {
        Map ((UINT64)PML4,Page->PhyAddr,Page->VirtAddr,Page->MemSiz,Page->Flgs | PE_P,MAP_4K);
    }
    Page--;

    /* 记得端掉 0x00 的老窝 */

    // PML4_Dump (PML4);

    Map ((UINT64)PML4,BASE_4KB,BASE_4KB,BASE_64MB - BASE_4KB,PE_P | PE_RW,MAP_4K);
    Map ((UINT64)PML4,BASE_64MB,BASE_64MB,BASE_256GB - BASE_64MB,PE_P | PE_RW,MAP_2M);

    *PML4Addr = (UINT64)PML4;

    return EFI_SUCCESS;
}

EFI_STATUS Map (
        IN UINT64 PML4Addr,
        IN UINT64 PhyAddr,
        IN UINT64 VirtAddr,
        IN UINT64 Siz,
        IN UINT64 Flgs,
        IN UINT8  Mode
        )
{
    ASSERT (PML4Addr != 0);
    ASSERT (Siz != 0);
    ASSERT (0 < Mode && Mode < MAP_ED);

    UINT64 PageSiz = (Mode == MAP_4K) ? SIZE_4KB : ((Mode == MAP_2M) ? SIZE_2MB : SIZE_1GB);

    PhyAddr = PhyAddr - PhyAddr % PageSiz;
    VirtAddr = VirtAddr - VirtAddr % PageSiz;
    UINT64 PageNum = SIZE_TO_PAGES(Siz,PageSiz);

    DEBUG ((DEBUG_INFO,"[INFO] Try to map 0x%llx -> 0x%llx - Mode : %u,size : %llu\n",PhyAddr,VirtAddr,Mode,Siz));

    PageTableProtect (FALSE); // Disable protection

    UINT64 *PML4 = (UINT64 *)PML4Addr;
    UINT64 *PDPT,*PD,*PT;
    for (UINTN Idx = 0;Idx < PageNum;Idx++,PhyAddr += PageSiz,VirtAddr += PageSiz)
    {
        /*
        WASTED METHOD (check it in next some operations):

        if (IsPageMapped (PML4Addr,VirtAddr))
        {
            DEBUG ((DEBUG_INFO,"[WARN] Page was mapped or dir was taken up! - Mode : %u - 0x%llx\n",Mode,VirtAddr));
            Break();
            goto Continue;
        }
        */

        if (~PML4[VIRT_PML4E_IDX(VirtAddr)] & PE_P)  // PDPT not found
        {
            PDPT = AllocatePages(1);
            ZeroMem (PDPT,SIZE_4KB);

            PML4[VIRT_PML4E_IDX(VirtAddr)] = PE_S_ADDR(PDPT) | PE_P | PE_RW;
        }
        else
        {
            PDPT = (UINT64 *)PE_V_ADDR(PML4[VIRT_PML4E_IDX(VirtAddr)]);
            ASSERT (PDPT != NULL);
        }

        /* Set a 1GiB page */
        if (Mode == MAP_1G)
        {
            PDPT[VIRT_PDPTE_IDX(VirtAddr)] = PE_S_ADDR(PhyAddr) | Flgs | PDPTE_1G;
            goto Continue;
        }

        if (~PDPT[VIRT_PDPTE_IDX(VirtAddr)] & PE_P)  // PD not found
        {
            PD = AllocatePages(1);
            ZeroMem (PD,SIZE_4KB);

            PDPT[VIRT_PDPTE_IDX(VirtAddr)] = PE_S_ADDR(PD) | PE_P | PE_RW;
        }
        else
        {
            PD = (UINT64 *)PE_V_ADDR(PDPT[VIRT_PDPTE_IDX(VirtAddr)]);
            ASSERT (PD != NULL);
        }

        /* Set a 2MiB page */
        if (Mode == MAP_2M)
        {
            PD[VIRT_PDE_IDX(VirtAddr)] = PE_S_ADDR(PhyAddr) | Flgs | PDE_2M;
            goto Continue;
        }

        if (~PD[VIRT_PDE_IDX(VirtAddr)] & PE_P)  // PT not found
        {
            PT = AllocatePages(1);
            ZeroMem (PT,SIZE_4KB);

            PD[VIRT_PDE_IDX(VirtAddr)] = PE_S_ADDR(PT) | PE_P | PE_RW;
        }
        else
        {
            PT = (UINT64 *)PE_V_ADDR(PD[VIRT_PDE_IDX(VirtAddr)]);
            ASSERT (PT != NULL);
        }

        /* Set a 4KiB page normally */
        PT[VIRT_PTE_IDX(VirtAddr)] = PE_S_ADDR(PhyAddr) | Flgs;

Continue:
        // DEBUG ((DEBUG_INFO,"[DONE] Mapped 0x%llx -> 0x%llx - size : %llu\n",PhyAddr,VirtAddr,PageSiz));
        PDPT = NULL;
        PD = NULL;
        PT = NULL;

        continue;
    }

    PageTableProtect (TRUE);

    return EFI_SUCCESS;
}

/* Update CR3 Register */
UINTN UpdateCr3 (
        IN UINTN  PML4Addr,
        IN UINT16 FlgsMask
        )
{
    PML4Addr &= ~0x7FF;
    FlgsMask &=  0x7FF;

    UINTN Flgs = AsmReadCr3() & 0x7FF;
    return AsmWriteCr3 (PML4Addr | (Flgs &~ FlgsMask));
}

/* 
   Check if this virtual address was mapped.
   
   @param  PML4Addr  The address of PML4 table.
   @param  VirtAddr  The address that will be checked.

   @retval BOOLEAN   The result.
*/
BOOLEAN IsPageMapped (
        IN UINT64 PML4Addr,
        IN UINT64 VirtAddr
        )
{    
    VirtAddr = VirtAddr - VirtAddr % SIZE_4KB;

    UINT64 *PML4 = (UINT64 *)PML4Addr;
    UINT64 *PDPT,*PD,*PT;
    if (~PML4[VIRT_PML4E_IDX(VirtAddr)] & PE_P)  // PDPT not found
    {
        goto NotFound;
    }
    else
    {
        PDPT = (UINT64 *)PE_V_ADDR(PML4[VIRT_PML4E_IDX(VirtAddr)]);
        ASSERT (PDPT != NULL);
    }

    if (~PDPT[VIRT_PDPTE_IDX(VirtAddr)] & PE_P)  // PD not found
    {
        goto NotFound;
    }
    else
    {
        if (PDPT[VIRT_PDPTE_IDX(VirtAddr)] & PDPTE_1G)
        {
            DEBUG ((DEBUG_INFO,"[INFO] Page was mapped - 0x%llx, Type : 1GiB\n",VirtAddr));
            return TRUE;
        }
        PD = (UINT64 *)PE_V_ADDR(PDPT[VIRT_PDPTE_IDX(VirtAddr)]);
        ASSERT (PD != NULL);
    }

    if (~PD[VIRT_PDE_IDX(VirtAddr)] & PE_P)  // PT not found
    {
        goto NotFound;
    }
    else
    {
        if (PD[VIRT_PDPTE_IDX(VirtAddr)] & PDE_2M)
        {
            DEBUG ((DEBUG_INFO,"[INFO] Page was mapped - 0x%llx, Type : 2MiB\n",VirtAddr));
            return TRUE;
        }
        PT = (UINT64 *)PE_V_ADDR(PD[VIRT_PDE_IDX(VirtAddr)]);
        ASSERT (PT != NULL);
    }

    if (~PT[VIRT_PTE_IDX(VirtAddr)] & PE_P)  // PG not found
    {
        goto NotFound;
    }

    DEBUG ((DEBUG_INFO,"[INFO] Mapped page was mapped - 0x%llx, Type : 4KiB\n",VirtAddr));
    return TRUE;

NotFound:
    DEBUG ((DEBUG_INFO,"[INFO] Mapped page was not mapped - 0x%llx\n",VirtAddr));
    return FALSE;
}

UINT64 VirtToPhyAddr (
        IN UINT64 PML4Addr,
        IN UINT64 VirtAddr
        )
{
    PML4Addr &= ~0x7FF;  // Aligned
    VirtAddr &= ~0x7FF;  // Aligned

    UINT64 *PML4 = (UINT64 *)PML4Addr;
    UINT64 *PDPT,*PD,*PT;

    /* If this page is not mapped,then return FALSE */
    if (!IsPageMapped (PML4Addr,VirtAddr))
    {
        return 0;
    }

    /* Locate the page step by step */
    PDPT = (UINT64 *)PE_V_ADDR(PML4[VIRT_PML4E_IDX(VirtAddr)]);
    if (PDPT[VIRT_PDPTE_IDX(VirtAddr)] & PDPTE_1G)
    {
        return PE_V_ADDR(PDPT[VIRT_PDPTE_IDX(VirtAddr)]);
    }
    
    PD   = (UINT64 *)PE_V_ADDR(PDPT[VIRT_PDPTE_IDX(VirtAddr)]);
    if (PD[VIRT_PDE_IDX(VirtAddr)] & PDE_2M)
    {
        return PE_V_ADDR(PD[VIRT_PDE_IDX(VirtAddr)]);
    }

    PT   = (UINT64 *)PE_V_ADDR(PD[VIRT_PDE_IDX(VirtAddr)]);

    return PE_V_ADDR(PT[VIRT_PTE_IDX(VirtAddr)]) | VIRT_OFFSET(VirtAddr);
}

VOID PML4_Dump (
        IN UINT64 *PML4
        )
{
    if (PML4 == NULL)
    {
        return;
    }
    UINT64 VirtAddr = 0;
    UINT64 *PDPT,*PD,*PT;
    for (UINT16 iPML4E = 0;iPML4E < 512;iPML4E++,PML4++)   // PML4 Level
    {
        if (~*PML4 & PE_P)
        {
            VirtAddr += PDPT_SIZE;
            continue;
        }
        PDPT = (UINT64 *)PE_V_ADDR(*PML4);
        for (UINT16 iPDPTE = 0;iPDPTE < 512;iPDPTE++,PDPT++)   // PDPT Level
        {
            if (~*PDPT & PE_P)
            {
                VirtAddr += PD_SIZE;
                continue;
            }

            if (*PDPT & PDPTE_1G)
            {
                DEBUG ((DEBUG_INFO,"[INFO] Mapped 0x%llx -> 0x%llx,Type : 1GiB\n",PE_V_ADDR(*PDPT),VirtAddr));
                VirtAddr += PD_SIZE;
                continue;
            }

            PD = (UINT64 *)PE_V_ADDR(*PDPT);
            for (UINT16 iPDE = 0;iPDE < 512;iPDE++,PD++)           // PD Level
            {
                if (~*PD & PE_P)
                {
                    VirtAddr += PT_SIZE;
                    continue;
                }

                if (*PD & PDE_2M)
                {
                    DEBUG ((DEBUG_INFO,"[INFO] Mapped 0x%llx -> 0x%llx,Type : 2MiB\n",PE_V_ADDR(*PD),VirtAddr));
                    VirtAddr += PT_SIZE;
                    continue;
                }

                PT = (UINT64 *)PE_V_ADDR(*PD);
                for (UINT16 iPTE = 0;iPTE < 512;iPTE++,PT++)       // PT Level
                {
                    if (~*PT & PE_P)
                    {
                        VirtAddr += PAGE_SIZE;
                        continue;
                    }

                    DEBUG ((DEBUG_INFO,"[INFO] Mapped 0x%llx -> 0x%llx,Type : 4KiB\n",PE_V_ADDR(*PT),VirtAddr));
                    VirtAddr += PAGE_SIZE;
                }
            }
        }
    }
}


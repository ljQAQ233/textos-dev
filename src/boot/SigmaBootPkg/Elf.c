#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Boot.h>
#include <Kernel.h>

EFI_STATUS ElfCheck (
        IN VOID* ElfBuffer
        )
{
    ELF_HEADER *Header = (ELF_HEADER*) ElfBuffer;
    if (Header->Magic != ELF_MAGIC)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Not an elf file\n"));
        return EFI_UNSUPPORTED;
    }

    if (Header->Type != ET_EXEC)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] This Elf type was wrong\n"));
        return EFI_UNSUPPORTED;
    }

    if (Header->Class != ELF_SUPPORTED_CLASS)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Unsupport the elf class\n"));
        return EFI_UNSUPPORTED;
    }

    if (Header->Machine != ELF_SUPPORTED_ARCH)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Unsupport elf machine type : %u , Arch : %u\n", Header->Machine, ELF_SUPPORTED_ARCH));
        return EFI_UNSUPPORTED;
    }
    DEBUG ((DEBUG_INFO ,"[ OK ] Elf is good!\n"));

    return EFI_SUCCESS;
}

EFI_STATUS ElfLoad (
        IN     VOID             *Buffer,
        IN     PHYSICAL_ADDRESS *Entry,
           OUT KERNEL_PAGE      **Pages
        )
{
    EFI_STATUS Status = EFI_SUCCESS;

    DEBUG ((DEBUG_INFO ,"[INFO] Loading elf...\n"));
    ERR_RETS (ElfCheck (Buffer));

    ELF_HEADER *Hdr = (ELF_HEADER *)Buffer;
    ELF_PHEADER *ProgHdr = Buffer + Hdr->PhOffset;
    DEBUG ((DEBUG_INFO ,"[INFO] Elf Program headers:\n"));

    UINTN LoadSegs = 0;
    for (UINTN i = 0;i< Hdr->PhNum;i++)
    {
        if (ProgHdr->Type == PT_LOAD)
        {
            LoadSegs++;
        }
        ProgHdr = (VOID *)ProgHdr + Hdr->PhentSiz;
    }
    DEBUG ((DEBUG_INFO ,"[INFO] The num of segments will be loaded : %llu\n",LoadSegs));

    UINTN PageNum = EFI_SIZE_TO_PAGES((LoadSegs + 1) * sizeof(KERNEL_PAGE));
    *Pages = AllocateReservedPages(PageNum);
    if (*Pages == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Fail;
    }
    gBS->SetMem (*Pages, EFI_PAGES_TO_SIZE(PageNum), 0);
    
    ProgHdr = Buffer + Hdr->PhOffset;
    KERNEL_PAGE *Page = *Pages;

    for (UINTN Idx = 0;Idx < Hdr->PhNum;Idx++,ProgHdr = (VOID *)ProgHdr + Hdr->PhentSiz)
    {
        if (ProgHdr->Type != PT_LOAD)
        {
            DEBUG ((DEBUG_INFO ,"       %u -> Isn't PT_LOAD\n",Idx));
            continue;
        }
        DEBUG ((DEBUG_INFO ,"       %u -> VirtAddr : 0x%llx,PhyAddr : 0x%llx\n", Idx, ProgHdr->VirtualAddr, ProgHdr->PhysicalAddr));
        DEBUG ((DEBUG_INFO ,"               FileSiz  : %llu,MemSiz  : %llu\n", ProgHdr->FileSiz, ProgHdr->MemSiz));
        
        VOID *Src = Buffer + ProgHdr->Offset;
        VOID *Dest = AllocateReservedPages(EFI_SIZE_TO_PAGES(ALIGN_VALUE(ProgHdr->MemSiz, ProgHdr->Align)));
        if (Dest == NULL) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Fail;
        }
        gBS->SetMem (Dest, ProgHdr->MemSiz, 0); // Padding
        gBS->CopyMem (
                Dest, Src,
                ProgHdr->FileSiz
                );

        if (ProgHdr->Flgs & PF_W)
        {
            Page->Flgs |= PE_RW;
        }
        if (ProgHdr->Flgs & PF_X)
        {
            // 默认可执行, 设置 NX 位不可执行
        }

        PHYSICAL_ADDRESS PhyAddr = (PHYSICAL_ADDRESS)Dest;

        Page->Valid = TRUE;
        Page->PhyAddr  = PhyAddr;
        Page->VirtAddr = ProgHdr->VirtualAddr;
        Page->MemSiz   = ALIGN_VALUE(ProgHdr->MemSiz,ProgHdr->Align);
        Page++;
    }
    *Entry = (PHYSICAL_ADDRESS)Hdr->Entry;

    RegisterMemory (PageNum, *Pages);

    return Status;

Fail:
    if (*Pages != NULL)
    {
        for (KERNEL_PAGE *Ptr = *Pages ; Ptr->Valid ; Ptr++) {
            ERR_RETS(gBS->FreePages (Ptr->PhyAddr, EFI_SIZE_TO_PAGES(Ptr->MemSiz)));
        }
    }

    return Status;
}

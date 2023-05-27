#include <Uefi.h>
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
    DEBUG ((DEBUG_INFO ,"[ OK ] Checked elf format for kernel successfully\n"));

    return EFI_SUCCESS;
}

EFI_STATUS ElfLoad (
        VOID             *Buffer,
        PHYSICAL_ADDRESS *Entry
        )
{
    DEBUG ((DEBUG_INFO ,"[INFO] Loading elf...\n"));
    ERR_RETS (ElfCheck (Buffer));

    ELF_HEADER *Hdr = (ELF_HEADER *)Buffer;
    ELF_PHEADER *PHdr = Buffer + Hdr->PhOffset;
    DEBUG ((DEBUG_INFO ,"[INFO] Elf Program headers:\n"));

    for (UINTN i = 0;i < Hdr->PhNum;i++)
    {
        if (PHdr->Type != PT_LOAD)
        {
            DEBUG ((DEBUG_INFO ,"       %u -> Isn't PT_LOAD\n",i));
            goto Continue;
        }
        DEBUG ((DEBUG_INFO ,"       %u -> VirtAddr : 0x%llx,PhyAddr : 0x%llx\n", i, PHdr->VirtualAddr, PHdr->PhysicalAddr));
        DEBUG ((DEBUG_INFO ,"               FileSiz  : %llu,MemSiz  : %llu\n", PHdr->FileSiz, PHdr->MemSiz));
        
        VOID *Src = Buffer + PHdr->Offset;
        VOID *Dest = (VOID *)PHdr->PhysicalAddr;
        gBS->SetMem (Dest,PHdr->MemSiz,0); // Padding

        gBS->CopyMem (
                Dest,Src,
                PHdr->FileSiz
                );

Continue:
        PHdr = (VOID *)PHdr + Hdr->PhentSiz;
    }
    *Entry = (PHYSICAL_ADDRESS)Hdr->Entry;

    return EFI_SUCCESS;
}

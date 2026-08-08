#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>

#include <Boot.h>
#include <Kernel.h>

EFI_STATUS
ElfCheck (
  IN VOID  *ElfBuffer
  )
{
  ELF_HEADER  *Header = (ELF_HEADER *)ElfBuffer;

  if (Header->Magic != ELF_MAGIC) {
    DEBUG ((DEBUG_ERROR, "[FAIL] Not an elf file\n"));
    return EFI_UNSUPPORTED;
  }

  if (Header->Type != ET_EXEC) {
    DEBUG ((DEBUG_ERROR, "[FAIL] This Elf type was wrong\n"));
    return EFI_UNSUPPORTED;
  }

  if (Header->Class != ELF_SUPPORTED_CLASS) {
    DEBUG ((DEBUG_ERROR, "[FAIL] Unsupport the elf class\n"));
    return EFI_UNSUPPORTED;
  }

  if (Header->Machine != ELF_SUPPORTED_ARCH) {
    DEBUG ((
      DEBUG_ERROR,
      "[FAIL] Unsupport elf machine type : %u , Arch : %u\n",
      Header->Machine,
      ELF_SUPPORTED_ARCH
      ));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "[ OK ] Elf is good!\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
ElfLoad (
  IN VOID              *Buffer,
  IN PHYSICAL_ADDRESS  *Entry,
  OUT KERNEL_PAGE      **Pages
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "[INFO] Loading elf...\n"));
  ERR_RETS (ElfCheck (Buffer));

  ELF_HEADER   *Header  = (ELF_HEADER *)Buffer;
  ELF_PHEADER  *PHeader = Buffer + Header->PhOffset;

  DEBUG ((DEBUG_INFO, "[INFO] Elf Program headers:\n"));

  UINTN  LoadableSegmentsCount = 0;

  for (UINTN i = 0; i < Header->PhNum; i++) {
    if (PHeader->Type == PT_LOAD) {
      LoadableSegmentsCount++;
    }

    PHeader = (VOID *)PHeader + Header->PhentSiz;
  }

  DEBUG ((
    DEBUG_INFO,
    "[INFO] The num of segments will be loaded : %llu\n",
    LoadableSegmentsCount
    ));

  UINTN  PageNum =
    EFI_SIZE_TO_PAGES ((LoadableSegmentsCount + 1) * sizeof (KERNEL_PAGE));

  *Pages = AllocateReservedPages (PageNum);
  if (*Pages == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Fail;
  }

  gBS->SetMem (*Pages, EFI_PAGES_TO_SIZE (PageNum), 0);

  PHeader = Buffer + Header->PhOffset;
  KERNEL_PAGE  *Page = *Pages;

  for (UINTN Index = 0; Index < Header->PhNum;
       Index++, PHeader = (VOID *)PHeader + Header->PhentSiz)
  {
    if (PHeader->Type != PT_LOAD) {
      DEBUG ((DEBUG_INFO, "       %u -> Isn't PT_LOAD\n", Index));
      continue;
    }

    DEBUG ((
      DEBUG_INFO,
      "       %u -> VirtAddr : 0x%llx,PhyAddr : 0x%llx\n",
      Index,
      PHeader->VirtualAddress,
      PHeader->PhysicalAddress
      ));
    DEBUG ((
      DEBUG_INFO,
      "               FileSiz  : %llu,MemSiz  : %llu\n",
      PHeader->FileSize,
      PHeader->MemSize
      ));

    VOID  *Source      = Buffer + PHeader->Offset;
    VOID  *Destination = AllocateReservedPages (
                           EFI_SIZE_TO_PAGES (
                             ALIGN_VALUE (PHeader->MemSize, PHeader->Alignment)
                             )
                           );
    if (Destination == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Fail;
    }

    gBS->SetMem (Destination, PHeader->MemSize, 0);    // Padding
    gBS->CopyMem (Destination, Source, PHeader->FileSize);

    if (PHeader->Flags & PF_W) {
      Page->Flgs |= PE_RW;
    }

    if (PHeader->Flags & PF_X) {
      // 默认可执行, 设置 NX 位不可执行
    }

    PHYSICAL_ADDRESS  PhysicalAddress = (PHYSICAL_ADDRESS)Destination;

    Page->IsValid         = TRUE;
    Page->PhysicalAddress = PhysicalAddress;
    Page->VirtualAddress  = PHeader->VirtualAddress;
    Page->MemorySize      = ALIGN_VALUE (PHeader->MemSize, PHeader->Alignment);
    Page++;
  }

  *Entry = (PHYSICAL_ADDRESS)Header->Entry;

  return Status;

Fail:
  if (*Pages != NULL) {
    for (KERNEL_PAGE *PageRecord = *Pages; PageRecord->IsValid;
         PageRecord++)
    {
      ERR_RETS (
        gBS->FreePages (
               PageRecord->PhysicalAddress,
               EFI_SIZE_TO_PAGES (PageRecord->MemorySize)
               )
        );
    }
  }

  return Status;
}

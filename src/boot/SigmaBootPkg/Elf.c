#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>

#include <Elf.h>
#include <Boot.h>
#include <Kernel.h>

EFI_STATUS
ElfCheck (
  IN VOID  *ElfBuffer
  )
{
  Elf_Ehdr  *Ehdr = (Elf_Ehdr *)ElfBuffer;

  if ((Ehdr->e_ident[0] != ELFMAG0) ||
      (Ehdr->e_ident[1] != ELFMAG1) ||
      (Ehdr->e_ident[2] != ELFMAG2) ||
      (Ehdr->e_ident[3] != ELFMAG3))
  {
    DEBUG ((DEBUG_ERROR, "[FAIL] Not an elf file\n"));
    return EFI_UNSUPPORTED;
  }

  if (Ehdr->e_type != ET_EXEC) {
    DEBUG ((DEBUG_ERROR, "[FAIL] This Elf type was wrong\n"));
    return EFI_UNSUPPORTED;
  }

  if (Ehdr->e_machine != ELF_SUPPORTED_ARCH) {
    DEBUG ((
      DEBUG_ERROR,
      "[FAIL] Unsupport elf machine type : %u , Arch : %u\n",
      Ehdr->e_machine,
      ELF_SUPPORTED_ARCH
      ));
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

#define ALIGN_UP(X, Y)    ((Y) * ((X + Y - 1) / Y))
#define ALIGN_DOWN(X, Y)  ((Y) * (X / Y))

EFI_STATUS
ElfLoad (
  IN VOID               *Buffer,
  OUT PHYSICAL_ADDRESS  *Entry,
  OUT PHYSICAL_ADDRESS  *Base,
  OUT UINT64            *Size
  )
{
  EFI_STATUS        Status           = EFI_SUCCESS;
  Elf_Ehdr          *Ehdr            = (Elf_Ehdr *)Buffer;
  VOID              *ProgramHdrs     = Buffer + Ehdr->e_phoff;
  Elf_Phdr          *Phdr            = Buffer + Ehdr->e_phoff;
  UINTN             LoadableSegments = 0;
  VOID              *LoadBase        = NULL;
  UINT64            LoadSize         = 0;
  UINT64            LoadOffset       = 0;
  PHYSICAL_ADDRESS  PhysicalMax      = 0;
  PHYSICAL_ADDRESS  PhysicalMin      = MAX_UINT64;
  PHYSICAL_ADDRESS  PhysicalStart;
  PHYSICAL_ADDRESS  PhysicalEnd;

  DEBUG ((DEBUG_INFO, "Loading elf...\n"));
  ERR_RETS (ElfCheck (Buffer));

  //
  // Actually elf conforms that PT_LOADs keep an ascending order. Since we need
  // to check all the program headers, we acquire PhysicalMin/Max in passing
  //
  for (UINT64 i = 0; i < Ehdr->e_phnum; i++) {
    Phdr = (Elf_Phdr *)(ProgramHdrs + Ehdr->e_phentsize * i);
    if (Phdr->p_type != PT_LOAD) {
      continue;
    }

    PhysicalStart = ALIGN_DOWN (Phdr->p_paddr, Phdr->p_align);
    PhysicalEnd   = ALIGN_UP (Phdr->p_paddr + Phdr->p_memsz, Phdr->p_align);

    LoadableSegments++;
    PhysicalMax = MAX (PhysicalMax, PhysicalEnd);
    PhysicalMin = MIN (PhysicalMin, PhysicalStart);
  }

  LoadSize   = PhysicalMax - PhysicalMin;
  LoadOffset = PhysicalMin;
  LoadBase   = AllocatePages (EFI_SIZE_TO_PAGES (LoadSize));
  if (LoadBase == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((
    DEBUG_INFO,
    " LoadBase = %lx\n, LoadSize = %lx"
    " %llu will be loaded:\n",
    LoadBase,
    LoadSize,
    LoadableSegments
    ));

  for (UINT64 i = 0; i < Ehdr->e_phnum; i++) {
    Phdr = (Elf_Phdr *)(ProgramHdrs + Ehdr->e_phentsize * i);
    if (Phdr->p_type != PT_LOAD) {
      continue;
    }

    CopyMem (
      LoadBase + Phdr->p_paddr - LoadOffset,
      Buffer + Phdr->p_offset,
      Phdr->p_filesz
      );
    SetMem (
      LoadBase + Phdr->p_paddr - LoadOffset + Phdr->p_filesz,
      Phdr->p_memsz - Phdr->p_filesz,
      0
      );
    DEBUG ((
      DEBUG_INFO,
      "  [#%lu] file(%lx, %lx) -> mem(%lx, %lx)\n",
      i,
      Phdr->p_offset,
      Phdr->p_filesz,
      (UINT64)LoadBase + Phdr->p_paddr - LoadOffset,
      Phdr->p_memsz
      ));
  }

  *Entry = (PHYSICAL_ADDRESS)LoadBase + Ehdr->e_entry - LoadOffset;
  *Base  = (PHYSICAL_ADDRESS)LoadBase;
  *Size  = LoadSize;
  DEBUG ((DEBUG_INFO, "Elf entry is at %lx\n", *Entry));

  return Status;
}

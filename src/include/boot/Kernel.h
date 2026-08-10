#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <Elf.h>

EFI_STATUS
KernelLoad (
  IN CHAR16                 *Path,
  OUT EFI_PHYSICAL_ADDRESS  *Entry,
  OUT EFI_PHYSICAL_ADDRESS  *Base,
  OUT UINT64                *Size
  );

#endif

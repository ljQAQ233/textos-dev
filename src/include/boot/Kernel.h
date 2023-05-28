#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <Elf.h>
#include <Page.h>

EFI_STATUS
KernelLoad (
  IN     CHAR16               *Path,
     OUT EFI_PHYSICAL_ADDRESS *Addr,
     OUT KERNEL_PAGE          **Pages
  );

#endif

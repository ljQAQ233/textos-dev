#ifndef __KERNEL_H__
#define __KERNEL_H__

EFI_STATUS
KernelLoad (
  IN     CHAR16               *Path,
     OUT EFI_PHYSICAL_ADDRESS *Addr
  );

#endif

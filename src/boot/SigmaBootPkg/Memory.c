#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>

#include <Boot.h>
#include <Memory.h>

EFI_STATUS
MemoryGetMap (
  MAP_INFO  *Info
  )
{
  ASSERT (Info != NULL);

  EFI_STATUS  Status  = EFI_SUCCESS;
  VOID        *Descs   = NULL;
  UINTN       MapSiz  = 0;
  UINTN       PageNum = 0;

Retry:
  Status = gBS->GetMemoryMap (
                  &MapSiz,
                  Descs,
                  &Info->MapKey,
                  &Info->DescSize,
                  &Info->DescVersion
                  );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    if (Descs != NULL) {
      gBS->FreePages ((EFI_PHYSICAL_ADDRESS)Descs, PageNum);
    }

    PageNum = EFI_SIZE_TO_PAGES (MapSiz);
    Status  = gBS->AllocatePages (
                     AllocateAnyPages,
                     EfiReservedMemoryType,
                     PageNum,
                     (EFI_PHYSICAL_ADDRESS *)&Descs
                     );
    ERR_RETS (Status);

    goto Retry;
  }

  ERR_RETS (Status);

  Info->Descs     = Descs;
  Info->MapSize   = MapSiz;
  Info->MapCount = Info->MapSize / Info->DescSize;
  return Status;
}

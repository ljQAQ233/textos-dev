#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Boot.h>
#include <Memory.h>

EFI_STATUS MemoryGetMap (MAP_INFO *Info)
{
    ASSERT (Info != NULL);

    EFI_STATUS Status = EFI_SUCCESS;
    VOID       *Maps = NULL;
    UINTN      MapSiz = 0;
    UINTN      PageNum = 0;

Retry:
    Status = gBS->GetMemoryMap (
            &MapSiz,
            Maps,
            &Info->MapKey,
            &Info->DescSiz,
            &Info->DescVersion
            );
    if (Status == EFI_BUFFER_TOO_SMALL) {
        if (Maps != NULL) {
            gBS->FreePages ((EFI_PHYSICAL_ADDRESS)Maps, PageNum);
        }

        PageNum = EFI_SIZE_TO_PAGES(MapSiz);
        Status = gBS->AllocatePages (
                AllocateAnyPages,
                EfiReservedMemoryType,
                PageNum,
                (EFI_PHYSICAL_ADDRESS *)&Maps
                );
        ERR_RETS(Status);

        goto Retry;
    }
    ERR_RETS(Status);

    Info->Maps = Maps;
    Info->MapSiz = MapSiz;
    Info->MapCount = Info->MapSiz / Info->DescSiz;
    return Status;
}


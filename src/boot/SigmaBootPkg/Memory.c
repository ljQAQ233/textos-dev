#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Boot.h>
#include <Memory.h>

EFI_STATUS MemoryGetMap (MAP_INFO *Info)
{
    ASSERT (Info != NULL);

    EFI_STATUS Status = EFI_SUCCESS;

    Info->Maps = NULL;
    Info->MapSiz = 0;
    Info->MapKey = 0;
    Info->DescSiz = 0;
    Info->DescVersion = 0;

    while (gBS->GetMemoryMap (&Info->MapSiz,Info->Maps,&Info->MapKey,&Info->DescSiz,&Info->DescVersion) == EFI_BUFFER_TOO_SMALL)
    {
        if (Info->Maps != NULL)
        {
            gBS->FreePool (Info->Maps);
        }

        Status = gBS->AllocatePool (EfiLoaderData,Info->MapSiz,(VOID **)&Info->Maps);
        if (EFI_ERROR (Status))
        {
            DEBUG ((DEBUG_ERROR ,"[FAIL] Failed to get memory space to save Map - Status : %r\n",Status));
            return Status;
        }
    }

    return Status;
}


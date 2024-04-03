#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Boot.h>
#include <File.h>

EFI_FILE_PROTOCOL *gFileProtocol = NULL;

EFI_STATUS InitializeFileServices ()
{
    EFI_STATUS Status = EFI_SUCCESS;

    UINTN      HandleCount = 0;
    EFI_HANDLE *HandleBuffer = NULL;

    Status = gBS->LocateHandleBuffer (
            ByProtocol,
            &gEfiSimpleFileSystemProtocolGuid,
            NULL,
            &HandleCount,&HandleBuffer
        );
    if (EFI_ERROR(Status))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] LocateHandleBuffer : gEfiSimpleFileSystemProtocolGuid - Status : %r\n",Status));
        return Status;
    }
    DEBUG ((DEBUG_INFO ,"[ OK ] LocateHandleBuffer - Count : %llu\n",HandleCount));

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    Status = gBS->OpenProtocol (
            HandleBuffer[0],
            &gEfiSimpleFileSystemProtocolGuid,
            (VOID**)&FileSystem,
            gImageHandle,
            NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );
    if (EFI_ERROR(Status))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] OpenProtocol : File Protocol - Status : %r\n",Status));
        return Status;
    }
    DEBUG ((DEBUG_INFO ,"[ OK ] Open FileProtocol\n"));
    gBS->FreePool (HandleBuffer);

    Status = FileSystem->OpenVolume (
            FileSystem,
            &gFileProtocol
        );
    if (EFI_ERROR(Status))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Open volume - Status : %r\n",Status));
        return Status;
    }
    DEBUG ((DEBUG_INFO ,"[ OK ] Opened volume successfully\n"));

    return Status;
}

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
            &HandleCount, &HandleBuffer
            );
    ERR_RETS(Status);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    Status = gBS->OpenProtocol (
            HandleBuffer[0],
            &gEfiSimpleFileSystemProtocolGuid,
            (VOID**)&FileSystem,
            gImageHandle,
            NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
            );
    ERR_RETS(Status);

    gBS->FreePool (HandleBuffer);
    Status = FileSystem->OpenVolume (
            FileSystem,
            &gFileProtocol
            );
    ERR_RETS(Status);

    return Status;
}

EFI_STATUS FileOpen (
        IN      CHAR16            *Path,
        IN      UINT64            Mode,
           OUT  EFI_FILE_PROTOCOL **File
    )
{
    EFI_STATUS Status = gFileProtocol->Open (
            gFileProtocol,
            File,
            Path,Mode,
            0
            );
    ERR_RETS(Status);

    return Status;
}

EFI_STATUS FileRead (
        IN     EFI_FILE_PROTOCOL *File,
           OUT VOID              *Data,
        IN OUT UINTN             *Size
        )
{
    EFI_STATUS Status = File->Read (
            File,
            Size,
            Data
            );
    ERR_RETS(Status);

    return Status;
}

EFI_STATUS FileWrite (
        IN     EFI_FILE_PROTOCOL *File,
        IN     VOID              *Buffer,
        IN OUT UINTN             *Size
        )
{
    EFI_STATUS Status = File->Write (
            File,
            Size,
            Buffer
            );
    ERR_RETS(Status);

    return Status;
}

EFI_STATUS FileFlush (EFI_FILE_PROTOCOL *File)
{
    EFI_STATUS Status = File->Flush (File);

    ERR_RETS(Status);

    return Status;
}

EFI_STATUS FileSetPosition (
        IN EFI_FILE_PROTOCOL    *File,
        IN UINT64               Position
        )
{
    EFI_STATUS Status = File->SetPosition (File,Position);
    
    ERR_RETS(Status);

    return Status;
}

UINT64 FileGetPosition (
        IN EFI_FILE_PROTOCOL *File
        )
{
    UINT64 Position = 0;

    EFI_STATUS Status = File->GetPosition (File,&Position);
    
    ERR_RETS(Status);

    return Position;
}


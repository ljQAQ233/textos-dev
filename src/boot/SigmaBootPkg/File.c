#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
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
            Path,Mode &~ O_NAPPEND,
            0
            );
    ERR_RETS(Status);

    DEBUG ((DEBUG_INFO ,"[ OK ] Open File : %S (%llx)\n", Path, Mode));

    if (Mode & O_NAPPEND)
    {
        DEBUG ((DEBUG_INFO ,"[INFO] Re-create file : %s\n", Path));
        ERR_RETS(FileRemove (*File)); // This operation includes delete and close!!!

        Mode |= O_CREATE;
        Mode &= ~O_NAPPEND;

        FileOpen (Path, Mode, File);
    }

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

EFI_STATUS FileAutoRead (
        IN     EFI_FILE_PROTOCOL *File,
           OUT VOID              **Data,
           OUT UINT64            *DataSize
        )
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_FILE_INFO *Info = NULL;

    Status = FileGetInfo (File,&Info);
    ERR_RETS(Status);
    
    DEBUG ((DEBUG_INFO ,"[ OK ] Get file infomation - %s\n",Info->FileName));

    /* Keep the `Info` unchanged */
    UINTN Size = Info->FileSize;
    *Data = AllocatePool (Size);
    if (Data == NULL)
    {
        DEBUG ((DEBUG_INFO, "[FAIL] Get memory space for the file is read\n"));
        return EFI_OUT_OF_RESOURCES;
    }
    
    Status = FileRead (File,*Data,&Size);
    ERR_RETS(Status);
    
    if (DataSize)
    {
        *DataSize = Size;
    }

    FileDestroyInfo (&Info);

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

EFI_STATUS FileGetInfo (
        IN     EFI_FILE_PROTOCOL *File,
           OUT EFI_FILE_INFO     **Info
        )
{
    EFI_STATUS Status = EFI_SUCCESS;

    /* Make the buffer a NULL that it returns a Correct size of the file info so that we allocate memory for that */
    UINTN Size = 0;
    *Info = (EFI_FILE_INFO *)NULL;
    
    Status = File->GetInfo (File,&gEfiFileInfoGuid,&Size,(VOID *)*Info);
    if (Status != EFI_BUFFER_TOO_SMALL)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Get the size of the file info - Status : %r\n",Status));
        return Status;
    }

    *Info = (EFI_FILE_INFO *)AllocatePool (Size);
    if (*Info == NULL)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Allocate memory for info\n"));
        return EFI_OUT_OF_RESOURCES;
    }

    Status = File->GetInfo (
            File,
            &gEfiFileInfoGuid,
            &Size, (VOID *)*Info
            );
    ERR_RETS(Status);

    return Status;
}

EFI_STATUS FileSetInfo (
        IN EFI_FILE_PROTOCOL *File,
        IN UINTN             Size,
        IN EFI_FILE_INFO     *Info
        )
{
    EFI_STATUS Status = File->SetInfo (
            File,
            &gEfiFileInfoGuid,
            Size, Info
            );
    ERR_RETS(Status);

    DEBUG ((DEBUG_INFO ,"[ OK ] Set info for this file - InfoSize : %llu\n",Size));
    return Status;
}

VOID FileDestroyInfo (EFI_FILE_INFO **Info)
{
    FreePool (*Info);
    *Info = NULL;
}

EFI_STATUS FileRemove (EFI_FILE_PROTOCOL *File)
{
    EFI_STATUS Status = File->Delete (File);

    ERR_RETS(Status);
    DEBUG ((DEBUG_INFO ,"[ OK ] Remove file\n"));

    return Status;
}

EFI_STATUS FileClose (EFI_FILE_PROTOCOL *File)
{
    EFI_STATUS Status = File->Close (File);

    ERR_RETS(Status);
    DEBUG ((DEBUG_INFO ,"[ OK ] Close file\n"));

    return Status;
}


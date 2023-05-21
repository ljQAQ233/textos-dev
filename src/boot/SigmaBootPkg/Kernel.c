#include <Uefi.h>

#include <Boot.h>
#include <File.h>
#include <Kernel.h>

EFI_STATUS KernelLoad (
        IN     CHAR16               *Path,
           OUT EFI_PHYSICAL_ADDRESS *Addr
        )
{
    EFI_FILE_PROTOCOL *File;
    ERR_RETS (FileOpen (Path, O_READ, &File));

    VOID *Binary;
    ERR_RETS (FileAutoRead (File, &Binary, NULL));

    *Addr = (EFI_PHYSICAL_ADDRESS)Binary;

    return EFI_SUCCESS;
}


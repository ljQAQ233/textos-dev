#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>

#include <Boot.h>
#include <File.h>
#include <Kernel.h>

EFI_STATUS KernelLoad (
        IN     CHAR16               *Path,
           OUT EFI_PHYSICAL_ADDRESS *Addr,
           OUT KERNEL_PAGE          **Pages
        )
{
    EFI_FILE_PROTOCOL *File;
    ERR_RETS (FileOpen (Path, O_READ, &File));

    VOID *Buffer;
    ERR_RETS (FileAutoRead (File, &Buffer, NULL));

    ERR_RETS (ElfLoad (Buffer, Addr, Pages));

    FreePool (Buffer);

    return EFI_SUCCESS;
}


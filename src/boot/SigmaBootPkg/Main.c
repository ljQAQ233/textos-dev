#include <Uefi.h>
#include <Library/UefiLib.h>

#include <Boot.h>
#include <Graphics.h>
#include <File.h>

EFI_STATUS EFIAPI UefiMain (
        IN EFI_HANDLE        ImageHandle,
        IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    Breakpoint();

    InitializeGraphicsServices();
    InitializeFileServices();

    EFI_FILE_PROTOCOL *File;
    FileOpen (L"\\Read.txt", O_READ | O_WRITE | O_CREATE, &File);

    EFI_FILE_INFO *Info = NULL;
    FileGetInfo (File, &Info);

    Print (L"Size of this file : %llu\n", Info->FileSize);

    VOID *Data;
    FileAutoRead (File, &Data, NULL);

    return EFI_SUCCESS;
}

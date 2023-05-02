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
    FileOpen (L"\\Read.txt", O_READ | O_WRITE | O_CREATE | O_NAPPEND, &File);

    CHAR8 *Buffer = "Hello world!!!";
    UINTN Size = AsciiStrLen (Buffer);

    FileWrite (File, Buffer, &Size);

    VOID *Data;
    FileAutoRead (File, &Data, NULL);

    return EFI_SUCCESS;
}

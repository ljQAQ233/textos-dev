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

    CHAR8 *Buffer = "Hello world!!!";
    UINTN Size = AsciiStrLen (Buffer);

    FileWrite (File,Buffer, &Size);
    Print (L"Write file!\n");

    FileRead (File,Buffer,&Size);
    Print (L"Read file and get data! - %a\n", Buffer);

    return EFI_SUCCESS;
}


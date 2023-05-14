#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

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

    GraphicsBmpDisplay (L"\\sigma1.bmp" , 0, 0);
    SystemTable->ConOut->ClearScreen (SystemTable->ConOut);
    GraphicsBmpDisplay (L"\\sigma4.bmp" , 0, 0);
    SystemTable->ConOut->ClearScreen (SystemTable->ConOut);
    GraphicsBmpDisplay (L"\\sigma8.bmp" , 0, 0);
    SystemTable->ConOut->ClearScreen (SystemTable->ConOut);
    GraphicsBmpDisplay (L"\\sigma24.bmp", 0, 0);
    SystemTable->ConOut->ClearScreen (SystemTable->ConOut);

    return EFI_SUCCESS;
}

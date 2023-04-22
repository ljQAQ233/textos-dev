#include <Uefi.h>

#include <Boot/Boot.h>

EFI_STATUS EFIAPI UefiMain (
        IN EFI_HANDLE        ImageHandle,
        IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    Breakpoint();
    SystemTable->ConOut->OutputString (
            SystemTable->ConOut,
            L"Hello world!\n"
            );

    return EFI_SUCCESS;
}

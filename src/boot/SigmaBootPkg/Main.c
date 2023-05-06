#include <Uefi.h>

#include <Boot.h>
#include <Graphics.h>

EFI_STATUS EFIAPI UefiMain (
        IN EFI_HANDLE        ImageHandle,
        IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    Breakpoint();

    InitializeGraphicsServices();


    return EFI_SUCCESS;
}

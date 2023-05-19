#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Boot.h>
#include <Logo.h>
#include <Config.h>
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

    InitializeConfig();

    CHAR16 *LogoPath = ConfigGetStringChar16 ("logo", D_LOGO_PATH);
    LogoShow (LogoPath);

    return EFI_SUCCESS;
}

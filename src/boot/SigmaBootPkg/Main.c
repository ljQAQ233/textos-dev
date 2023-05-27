#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Boot.h>
#include <Logo.h>
#include <Config.h>
#include <Graphics.h>
#include <File.h>
#include <Kernel.h>

EFI_STATUS EFIAPI UefiMain (
        IN EFI_HANDLE        ImageHandle,
        IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    Breakpoint();

    InitializeGraphicsServices();
    InitializeFileServices();

    InitializeConfig();

    EFI_PHYSICAL_ADDRESS KernelEntry;
    KernelLoad (L"\\Kernel.elf", &KernelEntry);

    UINT64 Ret = ((UINT64 (*)(VOID))KernelEntry)(); // A ptr to entry and call it to get status it returned
    DEBUG ((DEBUG_INFO ,"[INFO] Kernel returned : %llu\n", Ret));

    return EFI_SUCCESS;
}

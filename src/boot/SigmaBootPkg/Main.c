#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Boot.h>
#include <Logo.h>
#include <Config.h>
#include <Graphics.h>
#include <File.h>
#include <Memory.h>
#include <Kernel.h>

/* From tanyugang's Code,and I modified it,very thanks! */

EFI_STATUS ExitBootServices (
        IN     EFI_HANDLE ImageHandle,
           OUT MAP_INFO   *Info
    )
{
    EFI_STATUS Status = EFI_SUCCESS;

    Status = MemoryGetMap (Info);
    ERR_RETS(Status);

    Status = gBS->ExitBootServices (
            ImageHandle,
            Info->MapKey
        );
    ERR_RETS(Status);

    Info->MapCount = Info->MapSiz / Info->DescSiz;

    return Status;
}

EFI_STATUS EFIAPI UefiMain (
        IN EFI_HANDLE        ImageHandle,
        IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    Breakpoint();

    InitializeGraphicsServices();
    InitializeFileServices();

    InitializeConfig();

    CHAR16 *KernelPath = ConfigGetStringChar16 ("kernel", D_KERNEL_PATH);

    EFI_PHYSICAL_ADDRESS KernelEntry;
    KernelLoad (KernelPath, &KernelEntry);

    MAP_INFO Map;
    ExitBootServices (ImageHandle, &Map);

    UINT64 Ret = ((UINT64 (*)(VOID))KernelEntry)(); // A ptr to entry and call it to get status it returned
    IGNORE (Ret);

    return EFI_SUCCESS;
}

#include <Uefi.h>

#include <Boot/Boot.h>

VOID RetVoid()
{
    ERR_RET (EFI_OUT_OF_RESOURCES);
    DEBUG ((DEBUG_INFO ,"This will not be go there later\n"));
}

EFI_STATUS RetStatus()
{
    ERR_RETS (EFI_INVALID_PARAMETER);
    DEBUG ((DEBUG_INFO ,"This will not be go there later\n"));
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiMain (
        IN EFI_HANDLE        ImageHandle,
        IN EFI_SYSTEM_TABLE  *SystemTable
        )
{
    Breakpoint();

    RetVoid();
    RetStatus();

    DEBUG ((DEBUG_INFO,"Test DEBUG Macro !!!\n"));
    ASSERT (1 != 1);

    return EFI_SUCCESS;
}

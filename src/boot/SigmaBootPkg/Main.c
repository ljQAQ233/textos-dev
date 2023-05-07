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

    /* Blue 0 Green 0 Red 0 -> RED */
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL Template = { 0, 0, 255, 0 };
    gGraphicsOutputProtocol->Blt (
            gGraphicsOutputProtocol,
            &Template, EfiBltVideoFill,
            0, 0, 0, 0,
            16, 16,
            0
        );

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer = AllocatePool (16 * 16 * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
    gGraphicsOutputProtocol->Blt (
            gGraphicsOutputProtocol,
            BltBuffer, EfiBltVideoToBltBuffer,
            0, 0, 0, 0,
            16, 16,
            16
        );

    /* Hard Core!!! */
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Ptr = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)gGraphicsOutputProtocol->Mode->FrameBufferBase;
    for (int i = 0 ; i < gGraphicsOutputProtocol->Mode->FrameBufferSize / sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL) ; i++, Ptr++)
    {
        *Ptr = Template;
    }

    return EFI_SUCCESS;
}

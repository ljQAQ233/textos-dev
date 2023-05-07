#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Boot.h>
#include <Graphics.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutputProtocol = NULL;

EFI_STATUS EFIAPI InitializeGraphicsServices ()
{
    EFI_STATUS Status = EFI_SUCCESS;

    Status = gBS->LocateProtocol (
            &gEfiGraphicsOutputProtocolGuid,
            NULL,
            (VOID **)&gGraphicsOutputProtocol
        );
    ERR_RETS(Status);

    return Status;
}

/* Set the similar resolution. */
EFI_STATUS GraphicsResolutionSet (
        IN INTN Hor,
        IN INTN Ver
        )
{
    EFI_STATUS Status = EFI_SUCCESS;

    UINTN ModeSet = 0;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info = NULL;

    INTN HorSet = 0,
         VerSet = 0;
    INTN Prev = -1;

    for (UINTN ModeIndex = 0,Size ; ModeIndex < gGraphicsOutputProtocol->Mode->MaxMode ; ModeIndex++)
    {
        Status = gGraphicsOutputProtocol->QueryMode (
                gGraphicsOutputProtocol,
                ModeIndex,
                &Size,&Info
            );
        if (EFI_ERROR (Status))
        {
            DEBUG ((DEBUG_ERROR ,"[FAIL] Looked for Screen Mode - Status : %r\n", Status));
            FreePool (Info);
            return Status;
        }

        INT64 iHor = Info->HorizontalResolution,
              iVer = Info->VerticalResolution;

        /* Using "Manhattan Distance" */
        INT64 Current = ABS(Hor - iHor) + ABS(Ver - iVer);
        if (Current < Prev || Prev == -1)
        {
            Prev = Current;
            ModeSet = ModeIndex;
            HorSet  = iHor;
            VerSet  = iVer;
        }
        FreePool (Info);
    }
    DEBUG ((DEBUG_INFO ,"[ OK ] Looked for Screen Mode - Mode : %llu\n", ModeSet));
    
    Status = gGraphicsOutputProtocol->SetMode (gGraphicsOutputProtocol, ModeSet);
    ERR_RETS(Status);
    DEBUG ((DEBUG_INFO ,"[ OK ] Set Screen Mode - Hor : %llu,Ver : %llu\n", HorSet, VerSet));

    return Status;
}

EFI_STATUS GraphicsPutPixel (
        IN UINTN                         X,
        IN UINTN                         Y,
        IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color
        )
{
    return gGraphicsOutputProtocol->Blt (
            gGraphicsOutputProtocol,
            &Color, EfiBltVideoFill,
            0, 0, X, Y, 1, 1,
            0
            );
}


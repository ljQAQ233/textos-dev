#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Uefi.h>

#include <Boot.h>
#include <Graphics.h>

EFI_GRAPHICS_OUTPUT_PROTOCOL  *gGraphicsOutputProtocol = NULL;

EFI_STATUS EFIAPI
InitializeGraphicsServices (
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  Status = gBS->LocateProtocol (
                  &gEfiGraphicsOutputProtocolGuid,
                  NULL,
                  (VOID **)&gGraphicsOutputProtocol
                  );
  ERR_RETS (Status);

  return Status;
}

/* Set the similar resolution. */
EFI_STATUS
GraphicsResolutionSet (
  IN INTN  TargetHor,
  IN INTN  TargetVer
  )
{
  EFI_STATUS                            Status            = EFI_SUCCESS;
  UINTN                                 CurrentMode       = 0;
  INTN                                  CurrentHorizontal = 0;
  INTN                                  CurrentVertical   = 0;
  INTN                                  PreviousDistance  = -1;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *ModeInfo         = NULL;

  for (UINTN ModeIndex = 0, Size;
       ModeIndex < gGraphicsOutputProtocol->Mode->MaxMode; ModeIndex++)
  {
    Status = gGraphicsOutputProtocol->QueryMode (
                                        gGraphicsOutputProtocol,
                                        ModeIndex,
                                        &Size,
                                        &ModeInfo
                                        );
    if (EFI_ERROR (Status)) {
      DEBUG ((
        DEBUG_ERROR,
        "[FAIL] Looked for Screen Mode - Status : %r\n",
        Status
        ));
      FreePool (ModeInfo);
      return Status;
    }

    INT64  Horizontal = ModeInfo->HorizontalResolution,
           Vertical   = ModeInfo->VerticalResolution;

    /* Using "Manhattan Distance" */
    INT64  Distance =
      ABS (TargetHor - Horizontal) + ABS (TargetVer - Vertical);
    if ((Distance < PreviousDistance) || (PreviousDistance == -1)) {
      PreviousDistance  = Distance;
      CurrentMode       = ModeIndex;
      CurrentHorizontal = Horizontal;
      CurrentVertical   = Vertical;
    }

    FreePool (ModeInfo);
  }

  DEBUG ((
    DEBUG_INFO,
    "[ OK ] Looked for Screen Mode - Mode : %llu\n",
    CurrentMode
    ));

  Status =
    gGraphicsOutputProtocol->SetMode (gGraphicsOutputProtocol, CurrentMode);
  ERR_RETS (Status);
  DEBUG ((
    DEBUG_INFO,
    "[ OK ] Set Screen Mode - Hor : %llu,Ver : %llu\n",
    CurrentHorizontal,
    CurrentVertical
    ));

  return Status;
}

EFI_STATUS
GraphicsPutPixel (
  IN UINTN                          X,
  IN UINTN                          Y,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL  Color
  )
{
  return gGraphicsOutputProtocol->Blt (
                                    gGraphicsOutputProtocol,
                                    &Color,
                                    EfiBltVideoFill,
                                    0,
                                    0,
                                    X,
                                    Y,
                                    1,
                                    1,
                                    0
                                    );
}

EFI_STATUS
GraphicsBmpDisplay (
  IN CHAR16  *Path,
  IN UINT64  X,
  IN UINT64  Y,
  IN UINT64  Mode
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  BMP_INFO  Bmp;

  ERR_RETS (BmpInfoLoad (Path, &Bmp));

  UINT64  ScreenHorizontal =
    gGraphicsOutputProtocol->Mode->Info->HorizontalResolution;
  UINT64  ScreenVertical =
    gGraphicsOutputProtocol->Mode->Info->VerticalResolution;

  if (Mode & ~0b11111111) {
    DEBUG ((DEBUG_INFO, "[FAIL] Invalid parameter! - Mode : %llx\n", Mode));
    return EFI_INVALID_PARAMETER;
  }

  if (Mode & ModeNormal) {
    goto Show;
  }

  if (Mode & ModeCenter) {
    Mode |= ModeHorMiddle;
    Mode |= ModeVerMiddle;
  }

  if (Mode & ModeHorMiddle) {
    X = (ScreenHorizontal - Bmp.Header.Width) / 2;
  }

  if (Mode & ModeVerMiddle) {
    Y = (ScreenVertical - Bmp.Header.Height) / 2;
  }

  if (Mode & ModeLeft) {
    X = 0;
  } else if (Mode & ModeRight) {
    X = ScreenHorizontal - Bmp.Header.Width;
  }

  if (Mode & ModeTop) {
    Y = 0;
  } else if (Mode & ModeBottom) {
    Y = ScreenVertical - Bmp.Header.Height;
  }

Show:
  Status = gGraphicsOutputProtocol->Blt (
                                      gGraphicsOutputProtocol,
                                      Bmp.Pixels,
                                      EfiBltBufferToVideo,
                                      0,
                                      0,
                                      X,
                                      Y,
                                      Bmp.Header.Width,
                                      Bmp.Header.Height,
                                      0
                                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "[FAIL] Display the image using GOP failed - Status : %r\n",
      Status
      ));
    return Status;
  }

  DEBUG ((
    DEBUG_INFO,
    "[DONE] Display the image using GOP - X : %llu,Y : %llu\n",
    X,
    Y
    ));

  BmpInfoDestroy (&Bmp);

  return Status;
}

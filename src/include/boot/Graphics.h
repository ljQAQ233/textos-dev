#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <Protocol/GraphicsOutput.h>

#include <Bmp.h>

extern EFI_GRAPHICS_OUTPUT_PROTOCOL *gGraphicsOutputProtocol;

EFI_STATUS EFIAPI InitializeGraphicsServices();

/* Set the similar resolution. */
EFI_STATUS
GraphicsResolutionSet (
  IN INTN Hor,
  IN INTN Ver
  );

EFI_STATUS
GraphicsPutPixel (
  IN UINTN                         X,
  IN UINTN                         Y,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL Color
  );

enum BmpDisplayMode {
  ModeNormal    = 1,
  ModeHorMiddle = (1 << 1),
  ModeVerMiddle = (1 << 2),
  ModeTop       = (1 << 3),
  ModeLeft      = (1 << 4),
  ModeBottom    = (1 << 5),
  ModeRight     = (1 << 6),
  ModeCenter    = (1 << 7),
};

/* Show a Bmp image from a file */
EFI_STATUS
GraphicsBmpDisplay (
  IN CHAR16 *Path,
  IN UINT64 X,
  IN UINT64 Y,
  IN UINT64 Mode
  );

#endif

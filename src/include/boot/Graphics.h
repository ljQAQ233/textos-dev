#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

#include <Protocol/GraphicsOutput.h>

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

#endif

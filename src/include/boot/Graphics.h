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

/* Show a Bmp image from a file */
EFI_STATUS
GraphicsBmpDisplay (
  IN CHAR16 *Path,
  IN UINT64 X,
  IN UINT64 Y
  );

#endif

#ifndef __FONT_H__
#define __FONT_H__

#include <Graphics.h> // Keep the file can be included independently

typedef struct {
  UINT8  *Base;   // The address of bitmaps
  UINT8  Width;   // Width of font
  UINT8  Height;  // Height of font
  UINT64 Size;
} FONT_CONFIG;

EFI_STATUS
FontLoad (
  IN     CHAR16      *Path,
     OUT FONT_CONFIG *Font
  );

EFI_STATUS
FontShow (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL  *Gop,
  IN FONT_CONFIG                   Font,
  IN UINT8                         Code,
  IN UINT32                        X,
  IN UINT32                        Y,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL FGColor,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL BGColor
  );

#endif

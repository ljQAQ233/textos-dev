#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Boot.h>
#include <File.h>
#include <Graphics.h>

/* Bmp images format checker normally */
STATIC EFI_STATUS
BmpCheckFormat (
  BMP_IMAGE_HEADER  *Header
  )
{
  DEBUG ((DEBUG_INFO, "[INFO] Check bmp format\n"));

  if ((Header->CharB != 'B') ||
      (Header->CharM != 'M'))
  {
    Print (
      L"Invalid Bmp header format in header symbol - CharB = %c, CharM = %c\n",
      (UINTN)Header->CharB,
      (UINTN)Header->CharM
      );
    return EFI_INVALID_PARAMETER;
  }

  if (Header->Width <= 0) {
    Print (L"Invalid Bmp header format in pixel width - Width = %d\n", Header->Width);
    return EFI_INVALID_PARAMETER;
  }

  if ((Header->ImageBits != 1) &&
      (Header->ImageBits != 4) &&
      (Header->ImageBits != 8) &&
      (Header->ImageBits != 24) &&
      (Header->ImageBits != 32))
  {
    Print (L"Invalid Bmp header format in bits - ImageBits = %u\n", Header->ImageBits);
    return EFI_INVALID_PARAMETER;
  }

  if (Header->CompressionType != 0) {
    Print (L"Invalid Bmp CompressionType - CompressionType = %u\n", Header->CompressionType);
    return EFI_UNSUPPORTED;
  }

  if (Header->ImageOffset < sizeof (BMP_IMAGE_HEADER)) {
    Print (
      L"Invalid Bmp ImageOffset - ImageOffset = %u, minimum = %u\n",
      Header->ImageOffset,
      (UINT32)sizeof (BMP_IMAGE_HEADER)
      );
    return EFI_INVALID_PARAMETER;
  }

  if (Header->ImageOffset - sizeof (BMP_IMAGE_HEADER) <
      sizeof (BMP_COLOR_MAP) * ((Header->ImageBits == 24) ? 0 : (1ULL << Header->ImageBits)))
  {
    Print (L"Invalid Bmp color maps - ImageOffset = %u\n", Header->ImageOffset);
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_INFO, "[ OK ] This Bmp format is correct!\n"));

  return EFI_SUCCESS;
}

/*
  Make the bmp format be pixels can used to display on uefi.
*/
STATIC VOID
BmpTranslate (
  IN     VOID                        *Rawdata,
  OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Output
  )
{
  BMP_IMAGE_HEADER  *Header = (BMP_IMAGE_HEADER *)Rawdata;

  DEBUG ((
    DEBUG_INFO,
    "[INFO] Width : %d,Height : %d,ImageOffset : %d\n",
    Header->Width,
    Header->Height,
    Header->ImageOffset
    ));

  UINT8                          *RawIdx   = (UINT8 *)Rawdata + Header->ImageOffset;
  BMP_COLOR_MAP                  *ColorMap = (BMP_COLOR_MAP *)((UINT8 *)Rawdata + sizeof (BMP_IMAGE_HEADER));
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *OutIdx   = Output;

  BOOLEAN  HeightPositive = (BOOLEAN)(Header->Height > 0);

  for (INTN HeightIdx = HeightPositive ? Header->Height - 1 : 0
       ; HeightPositive ? (HeightIdx >= 0) : (HeightIdx < ABS (Header->Height))
       ; HeightPositive ? (HeightIdx--) : (HeightIdx++)
       )
  {
    for (UINTN WidthIdx = 0; WidthIdx < Header->Width; RawIdx++) {
      OutIdx = &Output[Header->Width * HeightIdx + WidthIdx];
      if (Header->ImageBits == 1) {
        for (UINT8 BitIdx = 0; (BitIdx < 8) && WidthIdx < Header->Width; BitIdx++, WidthIdx++) {
          OutIdx        = &Output[Header->Width * HeightIdx + WidthIdx];
          OutIdx->Blue  = ColorMap[((*RawIdx) >> (7 - BitIdx)) & 0b1].Blue;
          OutIdx->Green = ColorMap[((*RawIdx) >> (7 - BitIdx)) & 0b1].Green;
          OutIdx->Red   = ColorMap[((*RawIdx) >> (7 - BitIdx)) & 0b1].Red;
        }
      } else if (Header->ImageBits == 4) {
        OutIdx->Blue  = ColorMap[(*RawIdx) >> 4].Blue;
        OutIdx->Green = ColorMap[(*RawIdx) >> 4].Green;
        OutIdx->Red   = ColorMap[(*RawIdx) >> 4].Red;
        WidthIdx++;
        if (WidthIdx < Header->Width) {
          OutIdx        = &Output[Header->Width * HeightIdx + WidthIdx];
          OutIdx->Blue  = ColorMap[(*RawIdx) & 0b1111].Blue;
          OutIdx->Green = ColorMap[(*RawIdx) & 0b1111].Green;
          OutIdx->Red   = ColorMap[(*RawIdx) & 0b1111].Red;
          WidthIdx++;
        }
      } else if (Header->ImageBits == 8) {
        OutIdx->Blue  = ColorMap[*RawIdx].Blue;
        OutIdx->Green = ColorMap[*RawIdx].Green;
        OutIdx->Red   = ColorMap[*RawIdx].Red;
        WidthIdx++;
      } else if (Header->ImageBits == 24) {
        OutIdx->Blue  = *RawIdx++;
        OutIdx->Green = *RawIdx++;
        OutIdx->Red   = *RawIdx;
        WidthIdx++;
      }
    }
  }
}

/* Load a bmp image from file and make a BMP_INFO */
EFI_STATUS
BmpInfoLoad (
  CHAR16    *Path,
  BMP_INFO  *Bmp
  )
{
  EFI_STATUS  Status = EFI_SUCCESS;

  VOID               *Raw;
  EFI_FILE_PROTOCOL  *File;
  BMP_IMAGE_HEADER   Header;
  UINTN              ReadSize;

  ERR_RETS (FileOpen (Path, O_READ, &File));

  ReadSize = sizeof (BMP_IMAGE_HEADER);
  ERR_RETS (FileRead (File, &Header, &ReadSize));

  /* Check the format is good and supported */
  Status = BmpCheckFormat (&Header);
  if (EFI_ERROR (Status)) {
    Print (L"Unsupported BMP format or bad BMP! - Status : %r\n", Status);
    return Status;
  }

  DEBUG ((DEBUG_INFO, "[ OK ] Bmp is good!\n"));

  DEBUG ((DEBUG_INFO, "[INFO] Bmp format infomation\n"));
  DEBUG ((DEBUG_INFO, "       Bits : %u\n", Header.ImageBits));
  DEBUG ((DEBUG_INFO, "       Size : %llu,ImageSize : %llu\n", Header.Size, Header.ImageSize));
  DEBUG ((DEBUG_INFO, "       Width : %lld,Height : %lld\n", Header.Width, Header.Height));

  /* Get file raw */
  Raw      = AllocatePages (EFI_SIZE_TO_PAGES (Header.Size));
  ReadSize = Header.Size;
  ERR_RETS (FileSetPosition (File, 0));
  ERR_RETS (FileRead (File, Raw, &ReadSize));

  UINTN  Size = ABS (Header.Width) * ABS (Header.Height)
                * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  VOID  *Buffer = AllocatePages (EFI_SIZE_TO_PAGES (Size));

  if (Buffer == NULL) {
    Print (L"Failed to allocate memory for Buffer - Size = %llu\n", (UINT64)Size);
    return EFI_OUT_OF_RESOURCES;
  }

  BmpTranslate (Raw, Buffer);
  Bmp->Header = Header;
  Bmp->Size   = Size;
  Bmp->Pixels = Buffer;

  FreePages (Raw, EFI_SIZE_TO_PAGES (Header.Size));
  DEBUG ((DEBUG_INFO, "[ OK ] Updated all infomation and freed the used\n"));

  return Status;
}

VOID
BmpInfoDestroy (
  BMP_INFO  *Bmp
  )
{
  FreePages (Bmp->Pixels, EFI_SIZE_TO_PAGES (Bmp->Size));
  gBS->SetMem (Bmp, 0, sizeof (BMP_INFO));
}

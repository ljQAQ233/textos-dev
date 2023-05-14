#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Boot.h>
#include <File.h>
#include <Utils.h>
#include <Graphics.h>

/* Bmp images format checker normally */
STATIC EFI_STATUS BmpCheckFormat (BMP_IMAGE_HEADER *Hdr)
{
    DEBUG ((DEBUG_INFO, "[INFO] Check bmp format\n"));
    
    if (Hdr->CharB != 'B' || Hdr->CharM != 'M')
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invalid Bmp header format in header symbol\n"));
        return EFI_INVALID_PARAMETER;
    }

    if (Hdr->Width <= 0)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invalid Bmp header format in pixel width\n"));
        return EFI_INVALID_PARAMETER;
    }

    if (Hdr->ImageBits != 1 && Hdr->ImageBits != 4 && Hdr->ImageBits != 8 && Hdr->ImageBits != 24 && Hdr->ImageBits != 32)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invalid Bmp header format in bits\n"));
        return EFI_INVALID_PARAMETER;
    }

    if (Hdr->CompressionType != 0)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invalid Bmp CompressionType\n"));
        return EFI_UNSUPPORTED;
    }

    if (Hdr->ImageOffset < sizeof(BMP_IMAGE_HEADER))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invalid Bmp ImageOffset\n"));
        return EFI_INVALID_PARAMETER;
    }

    if (Hdr->ImageOffset - sizeof(BMP_IMAGE_HEADER) < sizeof(BMP_COLOR_MAP) * ((Hdr->ImageBits == 24) ? 0 : (PowerU64(2,Hdr->ImageBits))))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invalid Bmp color maps\n"));
        return EFI_INVALID_PARAMETER;
    }

    DEBUG ((DEBUG_INFO, "[ OK ] This Bmp format is correct!\n"));

    return EFI_SUCCESS;
}

/*
  Make the bmp format be pixels can used to display on uefi.
*/
STATIC VOID BmpTranslate (
        IN     VOID                           *Raw,
           OUT EFI_GRAPHICS_OUTPUT_BLT_PIXEL  *Out
        )
{
    BMP_IMAGE_HEADER *Hdr = (BMP_IMAGE_HEADER *)Raw;
    DEBUG ((DEBUG_INFO ,"[INFO] Width : %d,Height : %d,ImageOffset : %d\n", Hdr->Width, Hdr->Height, Hdr->ImageOffset));

    UINT8 *RawIdx = (UINT8 *)Raw + Hdr->ImageOffset;
    BMP_COLOR_MAP *ColorMap = (BMP_COLOR_MAP *)((UINT8 *)Raw + sizeof(BMP_IMAGE_HEADER));
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *OutIdx = Out;

    BOOLEAN HeightPositive = (BOOLEAN)(Hdr->Height > 0);

    for (INTN HeightIndex = HeightPositive ? Hdr->Height - 1 : 0
            ;HeightPositive ? (HeightIndex >= 0) : (HeightIndex < ABS(Hdr->Height))
            ;HeightPositive ? (HeightIndex--) : (HeightIndex++)
            )
    {
        for (UINTN WidthIdx = 0;WidthIdx < Hdr->Width;RawIdx++)
        {
            OutIdx = &Out[Hdr->Width * HeightIndex + WidthIdx];
            if (Hdr->ImageBits == 1)
            {
                for (UINT8 BitIndex = 0;(BitIndex < 8) && WidthIdx < Hdr->Width;BitIndex++,WidthIdx++)
                {
                    OutIdx = &Out[Hdr->Width * HeightIndex + WidthIdx];
                    OutIdx->Blue  = ColorMap[((*RawIdx) >> (7 - BitIndex)) & 0b1].Blue;
                    OutIdx->Green = ColorMap[((*RawIdx) >> (7 - BitIndex)) & 0b1].Green;
                    OutIdx->Red   = ColorMap[((*RawIdx) >> (7 - BitIndex)) & 0b1].Red;
                }
            }
            else if (Hdr->ImageBits == 4)
            {
                OutIdx->Blue  = ColorMap[(*RawIdx) >> 4].Blue;
                OutIdx->Green = ColorMap[(*RawIdx) >> 4].Green;
                OutIdx->Red   = ColorMap[(*RawIdx) >> 4].Red;
                WidthIdx++;
                if (WidthIdx < Hdr->Width)
                {
                    OutIdx = &Out[Hdr->Width * HeightIndex + WidthIdx];
                    OutIdx->Blue  = ColorMap[(*RawIdx) & 0b1111].Blue;
                    OutIdx->Green = ColorMap[(*RawIdx) & 0b1111].Green;
                    OutIdx->Red   = ColorMap[(*RawIdx) & 0b1111].Red;
                    WidthIdx++;
                }
            }
            else if (Hdr->ImageBits == 8)
            {
                OutIdx->Blue  = ColorMap[*RawIdx].Blue;
                OutIdx->Green = ColorMap[*RawIdx].Green;
                OutIdx->Red   = ColorMap[*RawIdx].Red;
                WidthIdx++;
            }
            else if (Hdr->ImageBits == 24)
            {
                OutIdx->Blue  = *RawIdx++;
                OutIdx->Green = *RawIdx++;
                OutIdx->Red   = *RawIdx;
                WidthIdx++;
            }
        }
    }
}

/* Load a bmp image from file and make a BMP_INFO */
EFI_STATUS BmpInfoLoad (
        CHAR16   *Path,
        BMP_INFO *Bmp
        )
{
    EFI_STATUS Status = EFI_SUCCESS;
    
    VOID *Raw;
    EFI_FILE_PROTOCOL *File;
    ERR_RETS (FileOpen (Path,O_READ,&File));

    BMP_IMAGE_HEADER Hdr;
    UINTN ReadSize = sizeof(BMP_IMAGE_HEADER);
    ERR_RETS (FileRead (File,&Hdr,&ReadSize));

    /* Check the format is good and supported */
    Status = BmpCheckFormat (&Hdr);
    if (EFI_ERROR(Status))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Unsupported BMP format or bad BMP!\n"));
        return Status;
    }
    DEBUG ((DEBUG_INFO ,"[DONE] Bmp is good!\n"));

    DEBUG ((DEBUG_INFO ,"[INFO] Bmp format infomation\n"));
    DEBUG ((DEBUG_INFO ,"       Bits : %u\n",Hdr.ImageBits));
    DEBUG ((DEBUG_INFO ,"       Size : %llu,ImageSize : %llu\n",Hdr.Size,Hdr.ImageSize));
    DEBUG ((DEBUG_INFO ,"       Width : %lld,Height : %lld\n",Hdr.Width,Hdr.Height));

    /* Get file raw */
    Raw = AllocatePages (EFI_SIZE_TO_PAGES(Hdr.Size));
    ReadSize = Hdr.Size;
    ERR_RETS (FileSetPosition (File,0));
    ERR_RETS (FileRead (File,Raw,&ReadSize));

    UINTN Size = ABS(Hdr.Width) * ABS(Hdr.Height) 
                  * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
    VOID *Buffer = AllocatePages (EFI_SIZE_TO_PAGES(Size));
    if (Buffer == NULL)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Allocated memory for Buffer failed\n"));
        return EFI_OUT_OF_RESOURCES;
    }

    BmpTranslate (Raw,Buffer);

    Bmp->Hdr = Hdr;
    Bmp->Size = Size;
    Bmp->Pixels = Buffer;

    FreePages (Raw,EFI_SIZE_TO_PAGES(Hdr.Size));
    DEBUG ((DEBUG_INFO ,"[DONE] Updated all infomation and freed the used\n"));

    return Status;
}

VOID BmpInfoDestroy (
        BMP_INFO *Bmp
        )
{
    FreePages (Bmp->Pixels,EFI_SIZE_TO_PAGES(Bmp->Size));
    gBS->SetMem (Bmp,0,sizeof(BMP_INFO));
}


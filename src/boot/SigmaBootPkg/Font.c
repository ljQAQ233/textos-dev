#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>

#include <Boot.h>
#include <File.h>
#include <Font.h>
#include <Config.h>
#include <Graphics.h>

EFI_STATUS FontLoad (
        IN     CHAR16      *Path,
           OUT FONT_CONFIG *Font
        )
{
    EFI_STATUS Status = EFI_SUCCESS;

    EFI_FILE_PROTOCOL *FontFile;
    Status = FileOpen (
            Path,
            EFI_FILE_MODE_READ,
            &FontFile
            );
    ERR_RETS(Status);

    Status = FileAutoRead (
            FontFile,
            (VOID**)&Font->Base,
            &Font->Size
            );
    ERR_RETS(Status);

    Font->Width  = ConfigGetNumUint64 ("font_width",0);
    Font->Height = ConfigGetNumUint64 ("font_height",0);

    if (Font->Height == 0 || Font->Width == 0)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invalid Font Width or Height\n"));
        return Status;
    }

    return Status;
}

EFI_STATUS FontShow (
        IN EFI_GRAPHICS_OUTPUT_PROTOCOL  *Gop,
        IN FONT_CONFIG                   Font,
        IN UINT8                         Code,
        IN UINT32                        X,
        IN UINT32                        Y,
        IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL FGColor,
        IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL BGColor
        )
{
    EFI_STATUS Status = EFI_SUCCESS;

    if (Code < 32 || Code > 126)
    {
        DEBUG ((DEBUG_WARN ,"[WARN] Invalid Char \"%u\" to print\n", Code));
    }

    UINTN PerBmpSiz = Font.Width * Font.Height / 8;
    /* Locate the address of start of bitmap,this pointer points the start of bitmap data,the size of bitmap in betys is (Width * Height / 8) */
    UINT8* Base = Font.Base + Code * PerBmpSiz;
    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *Pixel = AllocatePages (EFI_SIZE_TO_PAGES (PerBmpSiz));

    EFI_GRAPHICS_OUTPUT_BLT_PIXEL *PixelPtr = Pixel;
    for (UINT64 Idx = 0;Idx < PerBmpSiz;Idx++)
    {
        for (UINT8 BitIdx = 0;BitIdx < 8;BitIdx++)
        {
            if (((Base[Idx] >> (7 - BitIdx)) & 0b0000001) == 1)
            {
                *PixelPtr = FGColor;
            }
            else
            {
                *PixelPtr = BGColor;
            }
            PixelPtr++;
        }
    }

    Status = Gop->Blt (
            Gop,
            Pixel, EfiBltBufferToVideo,
            0, 0, X, Y, Font.Width, Font.Height,
            0
            );

    FreePages (Pixel,EFI_SIZE_TO_PAGES (PerBmpSiz));

    return Status;
}


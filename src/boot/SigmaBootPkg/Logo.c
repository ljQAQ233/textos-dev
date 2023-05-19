#include <Uefi.h>

#include <Boot.h>
#include <Graphics.h>

EFI_STATUS LogoShow (CHAR16 *Path)
{
    EFI_STATUS Status  = GraphicsBmpDisplay (Path, 0, 0, ModeCenter);
    if (EFI_ERROR(Status))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Unable to show logo(Bmp) : %S\n",Path));
        return Status;
    }

    return Status;
}

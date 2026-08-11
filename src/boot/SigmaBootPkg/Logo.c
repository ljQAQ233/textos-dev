#include <Library/UefiLib.h>
#include <Uefi.h>

#include <Boot.h>
#include <Graphics.h>

EFI_STATUS
LogoShow (
  CHAR16  *Path
  )
{
  EFI_STATUS  Status = GraphicsBmpDisplay (Path, 0, 0, ModeCenter);

  if (EFI_ERROR (Status)) {
    Print (L"Unable to show logo(Bmp) - Path : %S, Status : %r\n", Path, Status);
    return Status;
  }

  return Status;
}

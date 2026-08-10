#include <Library/MemoryAllocationLib.h>
#include <Uefi.h>

#include <Boot.h>
#include <File.h>
#include <Kernel.h>

EFI_STATUS
KernelLoad (
  IN CHAR16                 *Path,
  OUT EFI_PHYSICAL_ADDRESS  *Entry,
  OUT EFI_PHYSICAL_ADDRESS  *Base,
  OUT UINT64                *Size
  )
{
  VOID               *Buffer;
  EFI_FILE_PROTOCOL  *File;

  ERR_RETS (FileOpen (Path, O_READ, &File));
  ERR_RETS (FileAutoRead (File, &Buffer, NULL));
  ERR_RETS (ElfLoad (Buffer, Entry, Base, Size));

  FreePool (Buffer);

  return EFI_SUCCESS;
}

#ifndef __FILE_H__
#define __FILE_H__

#include <Protocol/SimpleFileSystem.h>

extern EFI_FILE_PROTOCOL *gFileProtocol;

EFI_STATUS InitializeFileServices ();

#define O_READ   EFI_FILE_MODE_READ
#define O_WRITE  EFI_FILE_MODE_WRITE
#define O_CREATE EFI_FILE_MODE_CREATE

EFI_STATUS
FileOpen (
  IN      CHAR16             *Path,
  IN      UINT64             Mode,
     OUT  EFI_FILE_PROTOCOL  **File
  );

EFI_STATUS
FileRead (
  IN      EFI_FILE_PROTOCOL  *File,
     OUT  VOID               *Data,
  IN OUT  UINTN              *Size
  );

EFI_STATUS
FileWrite (
  IN      EFI_FILE_PROTOCOL  *File,
  IN      VOID               *Buffer,
  IN OUT  UINTN              *Size
  );

EFI_STATUS
FileFlush (
  EFI_FILE_PROTOCOL *File
  );

EFI_STATUS
FileSetPosition (
  IN EFI_FILE_PROTOCOL    *File,
  IN UINT64               Position
  );

UINT64
FileGetPosition (
  IN EFI_FILE_PROTOCOL *File
  );

#endif

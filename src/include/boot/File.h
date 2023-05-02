#ifndef __FILE_H__
#define __FILE_H__

#include <Protocol/SimpleFileSystem.h>

#include <Guid/FileInfo.h>

extern EFI_FILE_PROTOCOL *gFileProtocol;

EFI_STATUS InitializeFileServices ();

#define O_READ   EFI_FILE_MODE_READ
#define O_WRITE  EFI_FILE_MODE_WRITE
#define O_CREATE EFI_FILE_MODE_CREATE

#define O_NAPPEND 0x4000000000000000ULL // 不追加,直接覆盖

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
FileAutoRead (
  IN     EFI_FILE_PROTOCOL *File,
     OUT VOID              **Data,
     OUT UINT64            *DataSize
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

EFI_STATUS
FileGetInfo (
  IN     EFI_FILE_PROTOCOL *File,
     OUT EFI_FILE_INFO     **Info
  );

EFI_STATUS FileSetInfo (
  IN EFI_FILE_PROTOCOL *File,
  IN UINTN             Size,
  IN EFI_FILE_INFO     *Info
  );

VOID FileDestroyInfo (EFI_FILE_INFO **Info);

EFI_STATUS FileRemove (EFI_FILE_PROTOCOL *File);

EFI_STATUS FileClose (EFI_FILE_PROTOCOL *File);

#endif

#ifndef __INI_H__
#define __INI_H__

typedef struct KV_TABLE {
  CHAR8          *Key;
  CHAR8          *Value;
  struct KV_TABLE *Next;
} KV_TABLE;

typedef struct {
  UINT32   Count;
  KV_TABLE *Kvs;
} INI_INFO;

EFI_STATUS
IniLoad (
  IN     CHAR16    *Path,
     OUT INI_INFO  *IniInfo
  );

EFI_STATUS
IniGetStringChar16S (
  IN     INI_INFO *Ini,
  IN     CHAR8    *Key,
  IN     CHAR8    *Default,
     OUT CHAR16   **String
  );

CHAR16
*IniGetStringChar16 (
  IN INI_INFO *Ini,
  IN CHAR8    *Key,
  IN CHAR8    *Default
  );

EFI_STATUS
IniGetNumUint64S (
  IN     INI_INFO *Ini,
  IN     CHAR8    *Key,
  IN     UINT64   Default,
     OUT UINT64   *Number
  );

UINT64
IniGetNumUint64 (
  IN INI_INFO *Ini,
  IN CHAR8    *Key,
  IN UINT64   Default
  );

#endif

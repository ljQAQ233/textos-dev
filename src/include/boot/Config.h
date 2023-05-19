#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Ini.h>

EFI_STATUS InitializeConfig ();

EFI_STATUS
ConfigGetStringChar16S (
  IN     CHAR8    *Key,
  IN     CHAR8    *Default,
     OUT CHAR16   **String
  );

CHAR16
*ConfigGetStringChar16 (
  IN CHAR8    *Key,
  IN CHAR8    *Default
  );

EFI_STATUS
ConfigGetNumUint64S (
  IN     CHAR8    *Key,
  IN     UINT64   Default,
     OUT UINT64   *Number
  );

UINT64
ConfigGetNumUint64 (
  IN CHAR8    *Key,
  IN UINT64   Default
  );

#endif

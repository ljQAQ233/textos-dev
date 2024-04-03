#ifndef __FILE_H__
#define __FILE_H__

#include <Protocol/SimpleFileSystem.h>

extern EFI_FILE_PROTOCOL *gFileProtocol;

EFI_STATUS InitializeFileServices ();

#endif

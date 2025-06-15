#ifndef __UEFI_H__
#define __UEFI_H__

// uefi
typedef int64_t   INT64;
typedef int32_t   INT32;
typedef int16_t   INT16;
typedef int8_t    INT8;

typedef uint64_t  UINT64;
typedef uint32_t  UINT32;
typedef uint16_t  UINT16;
typedef uint8_t   UINT8;

typedef intn_t   INTN;
typedef uintn_t  UINTN;

typedef char8   CHAR8;
typedef char16  CHAR16;

typedef uint8_t BOOLEAN;

typedef uintptr_t EFI_PHYSICAL_ADDRESS;
typedef uintptr_t EFI_VIRTUAL_ADDRESS;

#define TRUE     true
#define FALSE    false

#define STATIC   static 
#define CONST    const  
#define VOID     void   

#include <uefi_types.h>
#include <textos/uefi/mm.h>
#include <textos/uefi/table.h>

#include <textos/uefi/lib.h>

#endif

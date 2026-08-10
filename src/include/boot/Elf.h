#ifndef __ELF_H__
#define __ELF_H__

#define uint8_t    UINT8
#define uint16_t   UINT16
#define uint32_t   UINT32
#define uint64_t   UINT64
#define int8_t     INT8
#define int16_t    INT16
#define int32_t    INT32
#define int64_t    INT64
#define uintptr_t  UINTN
#define intptr_t   INTN

#include <bits/elf.h>

/* Mutiple arch but not used now */
#if defined (MDE_CPU_X64) || defined (MDE_CPU_AARCH64)
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Phdr Elf_Phdr;
#elif defined (MDE_CPU_IA32) || defined (MDE_CPU_ARM)
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32 Elf_Phdr;
#endif

#if defined (MDE_CPU_X64)
#define ELF_SUPPORTED_ARCH   EM_X86_64
#define ELF_SUPPORTED_CLASS  ELFCLASS64
#elif defined (MDE_CPU_IA32)
#define ELF_SUPPORTED_ARCH   EM_386
#define ELF_SUPPORTED_CLASS  ELFCLASS32
#elif defined (MDE_CPU_AARCH64)
#define ELF_SUPPORTED_ARCH   EM_AARCH64
#define ELF_SUPPORTED_CLASS  ELFCLASS64
#elif defined (MDE_CPU_ARM)
#define ELF_SUPPORTED_ARCH   EM_ARM
#define ELF_SUPPORTED_CLASS  ELFCLASS32
#endif

EFI_STATUS
ElfLoad (
  IN VOID               *Buffer,
  OUT PHYSICAL_ADDRESS  *Entry,
  OUT PHYSICAL_ADDRESS  *Base,
  OUT UINT64            *Size
  );

#endif

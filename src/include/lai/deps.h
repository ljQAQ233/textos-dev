/*
 * definitions in need to support lai on TextOS
 */

#pragma once

// stdarg.h
#include <textos/args.h>

// stdint.h
typedef u8  uint8_t;
typedef u16 uint16_t;
typedef u32 uint32_t;
typedef u64 uint64_t;

typedef int8  int8_t;
typedef int16 int16_t;
typedef int32 int32_t;
typedef int64 int64_t;

typedef addr_t uintptr_t;

// stddef.h
#define offsetof(type, m) ((size_t) &((type *)0)->m)
#define UINT64_C(value)  (value ## ULL)

typedef struct _packed
{
    u8 addrspace; // 0 - system memory, 1 - system I/O
    u8 bit_width;
    u8 bit_offset;
    u8 reserved;
    u64 addr;
} acpi_address_t;

#ifndef __BOOT_H__
#define __BOOT_H__

#include <textos/boot/efi.h>
#include <textos/boot/mb1.h>

typedef enum
{
    BOOT_EFI,
    BOOT_MB1,
    BOOT_MB2,
} bmode_t;

bmode_t bmode_get();
void *binfo_get();

#define __bdata __attribute__((section(".boot.data")))
#define __bcode __attribute__((section(".boot.text")))

#endif

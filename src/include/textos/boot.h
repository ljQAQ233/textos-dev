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

#define BOOT_DEV_PATH_BOOT "/dev/hda1"
#define BOOT_DEV_PATH_ROOT "/dev/hda2"

#endif

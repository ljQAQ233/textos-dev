#ifndef __BOOT_H__
#define __BOOT_H__

#include <Library/DebugLib.h>

#ifdef __SRC_LEVEL_DEBUG
#define Breakpoint()  __asm__ volatile ("int $1")
#else
#define Breakpoint()
#endif

// Return the status when error occurs
#define ERR_RETS(Expression)                                          \
    do {                                                              \
        EFI_STATUS __Status = Expression;                             \
        if (EFI_ERROR(__Status)) {                                    \
            DEBUG((DEBUG_ERROR, "%a(%d) expr: " #Expression "- %r\n", \
                   __FILE__, __LINE__, __Status));                    \
            return __Status;                                          \
        }                                                             \
    } while (FALSE);

// For non-value returning functions
#define ERR_RET(Expression)                                            \
    do {                                                               \
        EFI_STATUS __Status = Expression;                              \
        if (EFI_ERROR(__Status)) {                                     \
            DEBUG((DEBUG_ERROR, "%a(%d) expr: " #Expression " - %r\n", \
                   __FILE__, __LINE__, __Status));                     \
            return;                                                    \
        }                                                              \
    } while (FALSE);

#define CONFIG_PATH  L"\\config.ini"

#define D_HOR  1024
#define D_VER  768

#define D_LOGO_PATH    "\\sigma.bmp"
#define D_FONT_PATH    "\\ASC16"
#define D_KERNEL_PATH  "\\kernel.elf"

#define KERNEL_BASE  0x100000

#include <textos/boot/efi.h>

#endif

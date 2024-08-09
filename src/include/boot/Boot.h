#ifndef __BOOT_H__
#define __BOOT_H__

#include <Library/DebugLib.h>

#ifdef __SRC_LEVEL_DEBUG
  #define Breakpoint() __asm__ volatile ("int $1")
#else
  #define Breakpoint()
#endif

// Return the status when error occurs
#define ERR_RETS(Expr)                                       \
    do {                                                     \
        EFI_STATUS Stat = Expr;                              \
        if (EFI_ERROR (Stat)) {                              \
            DEBUG ((DEBUG_ERROR, "[FAIL] %a(%d) Exe: " #Expr \
                    "- %r\n", __FILE__, __LINE__, Stat));    \
            return Stat;                                     \
        }                                                    \
    } while (FALSE);

// For non-value returning functions
#define ERR_RET(Expr)                                        \
    do {                                                     \
        EFI_STATUS Stat = Expr;                              \
        if (EFI_ERROR (Stat)) {                              \
            DEBUG ((DEBUG_ERROR, "[FAIL] %a(%d) Exe: " #Expr \
                    " - %r\n", __FILE__, __LINE__, Stat));   \
            return;                                          \
        }                                                    \
    } while (FALSE);

#define IGNORE(Var) \
    { while (FALSE && Var); }

#define CONFIG_PATH L"\\config.ini"

#define D_HOR 1024
#define D_VER 768

#define D_LOGO_PATH "\\sigma.bmp"
#define D_FONT_PATH "\\ASC16"
#define D_KERNEL_PATH "\\kernel.elf"

#define KERNEL_BASE 0x100000

VOID RegisterMemory (
        UINT64 Count,
        VOID   *Pointer
        );

#endif

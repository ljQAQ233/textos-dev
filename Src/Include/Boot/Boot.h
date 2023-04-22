#ifndef __BOOT_H__
#define __BOOT_H__

#ifdef __SRC_LEVEL_DEBUG
  #define Breakpoint() __asm__ volatile ("int $1")
#else
  #define Breakpoint()
#endif

#endif

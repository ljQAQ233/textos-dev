#ifndef _SYS_AUXV_H
#define _SYS_AUXV_H

#include <elf.h>

unsigned long getauxval(unsigned long type);

#endif

#pragma once

#include <arch.h>
#include <textos/base.h>

#define asm __asm__

typedef int (*main_t)(
        int argc,
        const char **argv,
        const char **envp
        );

#define MAIN(name)          \
    int name(               \
        int argc,           \
        const char **argv,  \
        const char **envp   \
        )


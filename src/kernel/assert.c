#include <textos/debug.h>

void assertk(
        const char *file,
        const u64  line,
        const bool state,
        const char *expr
        )
{
    if (!state) {
        dprintk(K_SYNC, "[%s:%d] Assert failed!!! -> %s\n", file, line, expr);
    } else {
        return;
    }

    __asm__ volatile(
        "cli\n"
        "hlt\n"
    );
}

#include <stdio.h>

__attribute__((noreturn))
void __assert_fail(const char *expr, const char *file, int line, const char *func)
{
    dprintf(2, "Assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
    __builtin_unreachable();
    // todo : abort
}

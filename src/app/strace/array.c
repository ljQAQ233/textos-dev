#include "strace.h"
#include <sys/uio.h>

// To write code like this magic requires your LOL (。ヘ°)
// deferred expansion: _DEFER(f)(...) keeps `f` unexpanded for the current
// scan; a later _EXPAND(...) gives it a fresh scan to expand in.
#define _EMPTY()
#define _DEFER(id)   id _EMPTY()
#define _EXPAND(...) __VA_ARGS__

#define _(N, et)                                      \
    struct type _tya_##N = {                          \
        .name = _STR(N),                              \
        .size = 0,                                    \
        .cls = CLASS_ARR,                             \
        .printer = arr_printer,                       \
        .ARR = {.elem_type = _DEFER(refty_type)(et)}, \
    };
_EXPAND(ARR_COMPOUNDS(_))

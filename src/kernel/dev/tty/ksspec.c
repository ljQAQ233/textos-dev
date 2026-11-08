#include <textos/dev/keys.h>
#include <textos/dev/tty/console.h>

int ksspec_chtty(keysym_t sym)
{
    bool alt = 0 != (sym & (KEY_S_LALT | KEY_S_RALT));
    bool ctrl = 0 != (sym & (KEY_S_LCTRL | KEY_S_RCTRL));
    bool fn = KEYTYP(sym) == 'f';
    int idx = KEYCHR(sym);
    if (alt && ctrl && fn && (MIN_NR_CONSOLES <= idx && idx <= MAX_NR_CONSOLES))
        return idx;
    return 0;
}


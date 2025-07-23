#include <textos/dev/keys.h>
#include <textos/klib/string.h>

#define NON 0
#define ANY (KEY_S_MASK &~ KEY_S_RETMSK)

#define ALT (KEY_S_LALT|KEY_S_RALT)
#define CTRL (KEY_S_LCTRL|KEY_S_RCTRL)
#define SHIFT (KEY_S_LSHIFT|KEY_S_RSHIFT)
#define SUPER (KEY_S_LSUPER|KEY_S_RSUPER)
#define COMPOSE (KEY_S_COMPOSE)

int need_upper(keysym_t ks)
{
    int f1 = (ks & KEY_S_CAPSLK) ? 1 : 0;
    int f2 = (ks & SHIFT) ? 1 : 0;
    return f1 ^ f2;
}

char kstoc(keysym_t ks)
{
    int state = ks & KEY_S_MASK;
    char chr = KEYCHR(ks);
    char type = KEYTYP(ks);

    if (type == 's')
    {
        if (chr >= 'a' && chr <= 'z')
            return need_upper(ks) ? chr - 'a' + 'A' : chr;

        if (state & SHIFT)
        {
            switch (chr)
            {
                case '1': return '!';
                case '2': return '@';
                case '3': return '#';
                case '4': return '$';
                case '5': return '%';
                case '6': return '^';
                case '7': return '&';
                case '8': return '*';
                case '9': return '(';
                case '0': return ')';
                case '-': return '_';
                case '=': return '+';
                case '[': return '{';
                case ']': return '}';
                case '\\': return '|';
                case ';': return ':';
                case '\'': return '"';
                case ',': return '<';
                case '.': return '>';
                case '/': return '?';
                case '`': return '~';
            }
        }
        return chr;
    } else if (type == 'd' || type == 'p')
        return chr;

    return 0;
}

typedef struct
{
    keysym_t key;
    keysym_t mod;
    const char *str;
    /*
    * appkey value:
    * 0: no value
    * > 0: keypad application mode enabled
    *   = 2: this entry is for numlk = 1
    * < 0: keypad application mode disabled
    * 
    * > 0 -> the sequence to be sent differs from that in the main kbd
    */
    int appkey;
    /*
    * appcursor value:
    * 0: no value
    * > 0: cursor application mode enabled
    * < 0: cursor application mode disabled
    */
    int appcursor;
} kseq_map_t;

static kseq_map_t kseq[] = {
    { KEY_UP,          SHIFT,    "\033[1;2A",  0,  0 },
    { KEY_UP,          ALT,      "\033[1;3A",  0,  0 },
    { KEY_UP,        SHIFT|ALT,  "\033[1;4A",  0,  0 },
    { KEY_UP,          CTRL,     "\033[1;5A",  0,  0 },
    { KEY_UP,          CTRL,     "\033[1;2A",  0,  0 },
    { KEY_UP,    CTRL|SHIFT,     "\033[1;6A",  0,  0 },
    { KEY_UP,    CTRL|ALT,       "\033[1;7A",  0,  0 },
    { KEY_UP,    CTRL|SHIFT|ALT, "\033[1;8A",  0,  0 },
    { KEY_UP,          ANY,      "\033[A",     0, -1 },
    { KEY_UP,          ANY,      "\033OA",     0, +1 },
    { KEY_DOWN,        SHIFT,    "\033[1;2B",  0,  0 },
    { KEY_DOWN,        ALT,      "\033[1;3B",  0,  0 },
    { KEY_DOWN,       SHIFT|ALT, "\033[1;4B",  0,  0 },
    { KEY_DOWN,        CTRL,     "\033[1;5B",  0,  0 },
    { KEY_DOWN,        CTRL,     "\033[1;2B",  0,  0 },
    { KEY_DOWN,  CTRL|SHIFT,     "\033[1;6B",  0,  0 },
    { KEY_DOWN,  CTRL|ALT,       "\033[1;7B",  0,  0 },
    { KEY_DOWN,  CTRL|SHIFT|ALT, "\033[1;8B",  0,  0 },
    { KEY_DOWN,        ANY,      "\033[B",     0, -1 },
    { KEY_DOWN,        ANY,      "\033OB",     0, +1 },
    { KEY_LEFT,        SHIFT,    "\033[1;2D",  0,  0 },
    { KEY_LEFT,        ALT,      "\033[1;3D",  0,  0 },
    { KEY_LEFT,       SHIFT|ALT, "\033[1;4D",  0,  0 },
    { KEY_LEFT,        CTRL,     "\033[1;5D",  0,  0 },
    { KEY_LEFT,        CTRL,     "\033[1;2D",  0,  0 },
    { KEY_LEFT,  CTRL|SHIFT,     "\033[1;6D",  0,  0 },
    { KEY_LEFT,  CTRL|ALT,       "\033[1;7D",  0,  0 },
    { KEY_LEFT,  CTRL|SHIFT|ALT, "\033[1;8D",  0,  0 },
    { KEY_LEFT,        ANY,      "\033[D",     0, -1 },
    { KEY_LEFT,        ANY,      "\033OD",     0, +1 },
    { KEY_RIGHT,       SHIFT,    "\033[1;2C",  0,  0 },
    { KEY_RIGHT,       ALT,      "\033[1;3C",  0,  0 },
    { KEY_RIGHT,      SHIFT|ALT, "\033[1;4C",  0,  0 },
    { KEY_RIGHT,       CTRL,     "\033[1;5C",  0,  0 },
    { KEY_RIGHT,       CTRL,     "\033[1;2C",  0,  0 },
    { KEY_RIGHT, CTRL|SHIFT,     "\033[1;6C",  0,  0 },
    { KEY_RIGHT, CTRL|ALT,       "\033[1;7C",  0,  0 },
    { KEY_RIGHT, CTRL|SHIFT|ALT, "\033[1;8C",  0,  0 },
    { KEY_RIGHT,       ANY,      "\033[C",     0, -1 },
    { KEY_RIGHT,       ANY,      "\033OC",     0, +1 },
    { KEY_TAB,         SHIFT,    "\033[Z",     0,  0 },
    { KEY_ENTER,       ALT,      "\033\r",     0,  0 },
    { KEY_ENTER,       ANY,      "\r",         0,  0 },
    { KEY_INSERT,      SHIFT,    "\033[4l",   -1,  0 },
    { KEY_INSERT,      SHIFT,    "\033[2;2~", +1,  0 },
    { KEY_INSERT,      CTRL,     "\033[L",    -1,  0 },
    { KEY_INSERT,      CTRL,     "\033[2;5~", +1,  0 },
    { KEY_INSERT,      ANY,      "\033[4h",   -1,  0 },
    { KEY_INSERT,      ANY,      "\033[2~",   +1,  0 },
    { KEY_DELETE,      CTRL,     "\033[M",    -1,  0 },
    { KEY_DELETE,      CTRL,     "\033[3;5~", +1,  0 },
    { KEY_DELETE,      SHIFT,    "\033[2K",   -1,  0 },
    { KEY_DELETE,      SHIFT,    "\033[3;2~", +1,  0 },
    { KEY_DELETE,      ANY,      "\033[P",    -1,  0 },
    { KEY_DELETE,      ANY,      "\033[3~",   +1,  0 },
    { KEY_BACKSPACE,   NON,      "\177",       0,  0 },
    { KEY_BACKSPACE,   ALT,      "\033\177",   0,  0 },
    { KEY_HOME,        SHIFT,    "\033[2J",    0, -1 },
    { KEY_HOME,        SHIFT,    "\033[1;2H",  0, +1 },
    { KEY_HOME,        ANY,      "\033[H",     0, -1 },
    { KEY_HOME,        ANY,      "\033[1~",    0, +1 },
    { KEY_END,         CTRL,     "\033[J",    -1,  0 },
    { KEY_END,         CTRL,     "\033[1;5F", +1,  0 },
    { KEY_END,         SHIFT,    "\033[K",    -1,  0 },
    { KEY_END,         SHIFT,    "\033[1;2F", +1,  0 },
    { KEY_END,         ANY,      "\033[4~",    0,  0 },
    { KEY_PAGEUP,      CTRL,     "\033[5;5~",  0,  0 },
    { KEY_PAGEUP,      SHIFT,    "\033[5;2~",  0,  0 },
    { KEY_PAGEUP,      ANY,      "\033[5~",    0,  0 },
    { KEY_PAGEDOWN,    CTRL,     "\033[6;5~",  0,  0 },
    { KEY_PAGEDOWN,    SHIFT,    "\033[6;2~",  0,  0 },
    { KEY_PAGEDOWN,    ANY,      "\033[6~",    0,  0 },

    { KEY_KP7, SHIFT, "\033[2J",    0, -1 },
    { KEY_KP7, SHIFT, "\033[1;2H",  0, +1 },
    { KEY_KP7, ANY,   "\033[H",     0, -1 },
    { KEY_KP7, ANY,   "\033[1~",    0, +1 },
    { KEY_KP8, ANY,   "\033Ox",    +1,  0 },
    { KEY_KP8, ANY,   "\033[A",     0, -1 },
    { KEY_KP8, ANY,   "\033OA",     0, +1 },
    { KEY_KP2, ANY,   "\033Or",    +1,  0 },
    { KEY_KP2, ANY,   "\033[B",     0, -1 },
    { KEY_KP2, ANY,   "\033OB",     0, +1 },
    { KEY_KP4, ANY,   "\033Ot",    +1,  0 },
    { KEY_KP4, ANY,   "\033[D",     0, -1 },
    { KEY_KP4, ANY,   "\033OD",     0, +1 },
    { KEY_KP4, ANY,   "\033Ov",    +1,  0 },
    { KEY_KP4, ANY,   "\033[C",     0, -1 },
    { KEY_KP4, ANY,   "\033OC",     0, +1 },
    { KEY_KP9, SHIFT, "\033[5;2~",  0,  0 },
    { KEY_KP9, ANY,   "\033[5~",    0,  0 },
    { KEY_KP5, ANY,   "\033[E",     0,  0 },
    { KEY_KP1, CTRL,  "\033[J",    -1,  0 },
    { KEY_KP1, CTRL,  "\033[1;5F", +1,  0 },
    { KEY_KP1, SHIFT, "\033[K",    -1,  0 },
    { KEY_KP1, SHIFT, "\033[1;2F", +1,  0 },
    { KEY_KP1, ANY,   "\033[4~",    0,  0 },
    { KEY_KP3, SHIFT, "\033[6;2~",  0,  0 },
    { KEY_KP3, ANY,   "\033[6~",    0,  0 },
    { KEY_KP0, SHIFT, "\033[2;2~", +1,  0 },
    { KEY_KP0, SHIFT, "\033[4l",   -1,  0 },
    { KEY_KP0, CTRL,  "\033[L",    -1,  0 },
    { KEY_KP0, CTRL,  "\033[2;5~", +1,  0 },
    { KEY_KP0, ANY,   "\033[4h",   -1,  0 },
    { KEY_KP0, ANY,   "\033[2~",   +1,  0 },
    { KEY_KPD, CTRL,  "\033[M",    -1,  0 },
    { KEY_KPD, CTRL,  "\033[3;5~", +1,  0 },
    { KEY_KPD, SHIFT, "\033[2K",   -1,  0 },
    { KEY_KPD, SHIFT, "\033[3;2~", +1,  0 },
    { KEY_KPD, ANY,   "\033[P",    -1,  0 },
    { KEY_KPD, ANY,   "\033[3~",   +1,  0 },
    { KEY_KPA, ANY,   "\033Oj",    +2,  0 },
    { KEY_KPP, ANY,   "\033Ok",    +2,  0 },
    { KEY_KPM, ANY,   "\033Om",    +2,  0 },
    { KEY_KPD, ANY,   "\033On",    +2,  0 },
    { KEY_KPS, ANY,   "\033Oo",    +2,  0 },
    { KEY_KP0, ANY,   "\033Op",    +2,  0 },
    { KEY_KP1, ANY,   "\033Oq",    +2,  0 },
    { KEY_KP2, ANY,   "\033Or",    +2,  0 },
    { KEY_KP3, ANY,   "\033Os",    +2,  0 },
    { KEY_KP4, ANY,   "\033Ot",    +2,  0 },
    { KEY_KP5, ANY,   "\033Ou",    +2,  0 },
    { KEY_KP6, ANY,   "\033Ov",    +2,  0 },
    { KEY_KP7, ANY,   "\033Ow",    +2,  0 },
    { KEY_KP8, ANY,   "\033Ox",    +2,  0 },
    { KEY_KP9, ANY,   "\033Oy",    +2,  0 },
    { KEY_KP_ENTER, ANY, "\033OM", +2,  0 },
    { KEY_KP_ENTER, ANY, "\r",     -1,  0 },

    { KEY_F1,  /* F13 */ SHIFT,   "\033[1;2P" },
    { KEY_F1,  /* F25 */ CTRL,    "\033[1;5P" },
    { KEY_F1,  /* F37 */ SUPER,   "\033[1;6P" },
    { KEY_F1,  /* F49 */ ALT,     "\033[1;3P" },
    { KEY_F1,  /* F61 */ COMPOSE, "\033[1;4P" },
    { KEY_F2,  /* F14 */ SHIFT,   "\033[1;2Q" },
    { KEY_F2,  /* F26 */ CTRL,    "\033[1;5Q" },
    { KEY_F2,  /* F38 */ SUPER,   "\033[1;6Q" },
    { KEY_F2,  /* F50 */ ALT,     "\033[1;3Q" },
    { KEY_F2,  /* F62 */ COMPOSE, "\033[1;4Q" },
    { KEY_F3,  /* F15 */ SHIFT,   "\033[1;2R" },
    { KEY_F3,  /* F27 */ CTRL,    "\033[1;5R" },
    { KEY_F3,  /* F39 */ SUPER,   "\033[1;6R" },
    { KEY_F3,  /* F51 */ ALT,     "\033[1;3R" },
    { KEY_F3,  /* F63 */ COMPOSE, "\033[1;4R" },
    { KEY_F4,  /* F16 */ SHIFT,   "\033[1;2S" },
    { KEY_F4,  /* F28 */ CTRL,    "\033[1;5S" },
    { KEY_F4,  /* F40 */ SUPER,   "\033[1;6S" },
    { KEY_F4,  /* F52 */ ALT,     "\033[1;3S" },
    { KEY_F5,  /* F17 */ SHIFT,   "\033[15;2~" },
    { KEY_F5,  /* F29 */ CTRL,    "\033[15;5~" },
    { KEY_F5,  /* F41 */ SUPER,   "\033[15;6~" },
    { KEY_F5,  /* F53 */ ALT,     "\033[15;3~" },
    { KEY_F6,  /* F18 */ SHIFT,   "\033[17;2~" },
    { KEY_F6,  /* F30 */ CTRL,    "\033[17;5~" },
    { KEY_F6,  /* F42 */ SUPER,   "\033[17;6~" },
    { KEY_F6,  /* F54 */ ALT,     "\033[17;3~" },
    { KEY_F7,  /* F19 */ SHIFT,   "\033[18;2~" },
    { KEY_F7,  /* F31 */ CTRL,    "\033[18;5~" },
    { KEY_F7,  /* F43 */ SUPER,   "\033[18;6~" },
    { KEY_F7,  /* F55 */ ALT,     "\033[18;3~" },
    { KEY_F8,  /* F20 */ SHIFT,   "\033[19;2~" },
    { KEY_F8,  /* F32 */ CTRL,    "\033[19;5~" },
    { KEY_F8,  /* F44 */ SUPER,   "\033[19;6~" },
    { KEY_F8,  /* F56 */ ALT,     "\033[19;3~" },
    { KEY_F9,  /* F21 */ SHIFT,   "\033[20;2~" },
    { KEY_F9,  /* F33 */ CTRL,    "\033[20;5~" },
    { KEY_F9,  /* F45 */ SUPER,   "\033[20;6~" },
    { KEY_F9,  /* F57 */ ALT,     "\033[20;3~" },
    { KEY_F10, /* F22 */ SHIFT,   "\033[21;2~" },
    { KEY_F10, /* F34 */ CTRL,    "\033[21;5~" },
    { KEY_F10, /* F46 */ SUPER,   "\033[21;6~" },
    { KEY_F10, /* F58 */ ALT,     "\033[21;3~" },
    { KEY_F11, /* F23 */ SHIFT,   "\033[23;2~" },
    { KEY_F11, /* F35 */ CTRL,    "\033[23;5~" },
    { KEY_F11, /* F47 */ SUPER,   "\033[23;6~" },
    { KEY_F11, /* F59 */ ALT,     "\033[23;3~" },
    { KEY_F12, /* F24 */ SHIFT,   "\033[24;2~" },
    { KEY_F12, /* F36 */ CTRL,    "\033[24;5~" },
    { KEY_F12, /* F48 */ SUPER,   "\033[24;6~" },
    { KEY_F12, /* F60 */ ALT,     "\033[24;3~" },

    { KEY_F1,  NON, "\033OP" },
    { KEY_F2,  NON, "\033OQ" },
    { KEY_F3,  NON, "\033OR" },
    { KEY_F4,  NON, "\033OS" },
    { KEY_F5,  NON, "\033[15~" },
    { KEY_F6,  NON, "\033[17~" },
    { KEY_F7,  NON, "\033[18~" },
    { KEY_F8,  NON, "\033[19~" },
    { KEY_F9,  NON, "\033[20~" },
    { KEY_F10, NON, "\033[21~" },
    { KEY_F11, NON, "\033[23~" },
    { KEY_F12, NON, "\033[24~" },
    { KEY_F13, NON, "\033[1;2P" },
    { KEY_F14, NON, "\033[1;2Q" },
    { KEY_F15, NON, "\033[1;2R" },
    { KEY_F16, NON, "\033[1;2S" },
    { KEY_F17, NON, "\033[15;2~" },
    { KEY_F18, NON, "\033[17;2~" },
    { KEY_F19, NON, "\033[18;2~" },
    { KEY_F20, NON, "\033[19;2~" },
    { KEY_F21, NON, "\033[20;2~" },
    { KEY_F22, NON, "\033[21;2~" },
    { KEY_F23, NON, "\033[23;2~" },
    { KEY_F24, NON, "\033[24;2~" },
    { KEY_F25, NON, "\033[1;5P" },
    { KEY_F26, NON, "\033[1;5Q" },
    { KEY_F27, NON, "\033[1;5R" },
    { KEY_F28, NON, "\033[1;5S" },
    { KEY_F29, NON, "\033[15;5~" },
    { KEY_F30, NON, "\033[17;5~" },
    { KEY_F31, NON, "\033[18;5~" },
    { KEY_F32, NON, "\033[19;5~" },
    { KEY_F33, NON, "\033[20;5~" },
    { KEY_F34, NON, "\033[21;5~" },
    { KEY_F35, NON, "\033[23;5~" },
    { .str = NULL }
};

static inline int match(keysym_t seq, uint sym)
{
    if (seq == ANY)
        return 1;

    keysym_t mod = 0;
    if (sym & ALT) mod |= ALT;
    if (sym & CTRL) mod |= CTRL;
    if (sym & SHIFT) mod |= SHIFT;
    if (sym & SUPER) mod |= SUPER;
    if (sym & COMPOSE) mod |= COMPOSE;
    return seq == mod;
}

// TODO: bind with console!!!
static inline int appcursor()
{
    return -1;
}

static inline int appkeypad()
{
    return -1;
}

/**
 * @brief convert a sym to ascii values with keyboard layout 'en_us'
 */
int kstoa(keysym_t sym, char *buf)
{
    char ch = kstoc(sym);
    if (ch >= 'a' && ch <= 'z' && (sym & (CTRL | ALT)))
    {
        if (sym & CTRL)
            ch &= 0x1f;
        if (sym & ALT)
            *buf++ = '\033';
        *buf++ = ch;
        *buf++ = '\0';
        return (sym & ALT) ? 2 : 1;
    }

    keysym_t key = sym &~ KEY_S_MASK;
    kseq_map_t *seq = kseq;
    for ( ; seq->str ; seq++)
    {
        if (seq->key != key)
            continue;
        if (!match(seq->mod, sym))
            continue;
        if (appkeypad() * seq->appkey < 0)
            continue;
        if (appcursor() * seq->appcursor < 0)
            continue;
        const char *s = seq->str;
        char *p = buf;
        while ((*p++ = *s++));
        return p - buf - 1;
    }

    *buf++ = ch;
    *buf = '\0';
    return 1;
}

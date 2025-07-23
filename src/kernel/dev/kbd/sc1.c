/*
 * scan code set 1
 */
#include <textos/dev/keys.h>

static keysym_t map[] = {
    KEY_NONE,
    KEY_ESC,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LBRACKET,
    KEY_RBRACKET,
    KEY_ENTER,
    KEY_S_LCTRL,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_S_LSHIFT,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_S_RSHIFT,
    KEY_KPA,
    KEY_S_LALT,
    KEY_SPACE,
    KEY_S_CAPSLK,
    KEY_F1,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_S_NUMLK,
    0, // scrlock
    KEY_KP7,
    KEY_KP8,
    KEY_KP9,
    KEY_KPM,
    KEY_KP4,
    KEY_KP5,
    KEY_KP6,
    KEY_KPP,
    KEY_KP1,
    KEY_KP2,
    KEY_KP3,
    KEY_KP0,
    KEY_KPD,
    0, 0, 0, // nul
    KEY_F11,
    KEY_F12,
};

static keysym_t emap[] = {
    [0x10] = KEY_PREV_TRACK,
    [0x19] = KEY_NEXT_TRACK,
    [0x1c] = KEY_KP_ENTER,
    [0x1d] = KEY_S_RCTRL,
    [0x20] = KEY_MUTE,
    [0x21] = KEY_CALC,
    [0x22] = KEY_PLAY,
    [0x24] = KEY_STOP,
    [0x2e] = KEY_VOL_DOWN,
    [0x30] = KEY_VOL_UP,
    [0x32] = KEY_WWW_HOME,
    [0x35] = KEY_KP_BACKSLASH,
    [0x47] = KEY_HOME,
    [0x48] = KEY_UP,
    [0x49] = KEY_PAGEUP,
    [0x4b] = KEY_LEFT,
    [0x4d] = KEY_RIGHT,
    [0x4f] = KEY_END,
    [0x50] = KEY_DOWN,
    [0x51] = KEY_PAGEDOWN,
    [0x52] = KEY_INSERT,
    [0x53] = KEY_DELETE,
    [0x5b] = KEY_S_LSUPER,
    [0x5c] = KEY_S_RSUPER,
    [0x5d] = KEY_S_APP,
    [0x5e] = KEY_POWER,
    [0x65] = KEY_WWW_SEARCH,
    [0x66] = KEY_WWW_FAVOR,
    [0x67] = KEY_WWW_REFRESH,
    [0x68] = KEY_WWW_STOP,
    [0x69] = KEY_WWW_FORWARD,
    [0x6a] = KEY_WWW_BACK,
    [0x6b] = KEY_MY_COMPUTER,
    [0x6c] = KEY_EMAIL,
    [0x6d] = KEY_SELECT,
};

enum sc1_state
{
    ST_NORMAL,
    ST_E0,
    ST_E1,
    ST_SEQ,
};

// TODO: dev isolation
static enum sc1_state stat = ST_NORMAL;
static u8 seq[6];
static u8 i = 0;

static const u8 prtscr_seq[] = { 0xe0, 0x2a, 0xe0, 0x37 };
static const u8 pause_seq[] = { 0xe1, 0x1d, 0x45, 0xe1, 0x9d, 0xc5 };

static inline void reset()
{
    stat = ST_NORMAL;
    i = 0; 
}

/**
 * @brief convert code to KEY_XXX. this function doesn't care about if it is a break code.
 *        if code refers to a meta key, the state flag is returned, the flag may be recorded
 *        by the caller. if code is not completed (e.g. E0 / E1), the current state will be
 *        stored and waiting for the feed next time, then KEY_S_WAITING is returned.
 */
keysym_t kbd_sc1_to_sym(u8 code)
{
    code &= 0x7f;
    switch (stat)
    {
    case ST_NORMAL:
        if (code == 0xe0) {
            stat = ST_E0;
            return KEY_S_WAIT;
        } else if (code == 0xe1) {
            stat = ST_E1;
            return KEY_S_WAIT;
        } else {
            if (code < sizeof(map) / sizeof(keysym_t) && map[code])
                return map[code];
            return KEY_S_ERROR;
        }

    case ST_E0:
        seq[0] = 0xe0;
        seq[1] = code;
        if (code == 0x2a) {
            i = 2;
            stat = ST_SEQ;
            return KEY_NONE;
        } else {
            stat = ST_NORMAL;
            if (code < sizeof(emap) / sizeof(keysym_t) && map[code])
                return emap[code];
            return KEY_S_ERROR;
        }

    case ST_E1:
        seq[0] = 0xe1;
        seq[1] = code;
        i = 2;
        stat = ST_SEQ;
        return KEY_S_WAIT;

    case ST_SEQ:
        if (i == sizeof(prtscr_seq))
        {
            for (int j = 0 ; j < i ; j++)
                if (seq[j] != prtscr_seq[j]) {
                    reset();
                    return KEY_S_ERROR;
                }
            return KEY_PRTSCR;
        }
        if (i == sizeof(pause_seq))
        {
            for (int j = 0 ; j < i ; j++)
                if (seq[j] != pause_seq[j]) {
                    reset();
                    return KEY_S_ERROR;
                }
            return KEY_PAUSE;
        }
        return KEY_S_WAIT;
    }
    reset();
    return KEY_S_ERROR;
}

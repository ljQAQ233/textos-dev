// gcc docs/kernel/graphics/acsn.c -lncurses && TERM=linux ./a.out
// nix-shell -p ncurses gcc --run \
//   'gcc docs/kernel/graphics/acsn.c -lncurses && TERM=linux ./a.out'
#include <ncurses.h>

// clang-format off
#define ACS_LIST \
    X(ACS_BLOCK)    X(ACS_BOARD) \
    X(ACS_BTEE)     X(ACS_BULLET) \
    X(ACS_CKBOARD)  X(ACS_DARROW) \
    X(ACS_DEGREE)   X(ACS_DIAMOND) \
    X(ACS_HLINE)    X(ACS_LANTERN) \
    X(ACS_LARROW)   X(ACS_LLCORNER) \
    X(ACS_LRCORNER) X(ACS_LTEE) \
    X(ACS_PLMINUS)  X(ACS_PLUS) \
    X(ACS_RARROW)   X(ACS_RTEE) \
    X(ACS_S1)       X(ACS_S9) \
    X(ACS_TTEE)     X(ACS_UARROW) \
    X(ACS_ULCORNER) X(ACS_URCORNER) \
    X(ACS_VLINE)    X(ACS_GEQUAL) \
    X(ACS_LEQUAL)   X(ACS_NEQUAL) \
    X(ACS_PI)       X(ACS_S3) \
    X(ACS_S7)       X(ACS_STERLING)
// clang-format on

int main(void)
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    const struct
    {
        chtype ch;
        const char *name;
    } acs[] = {
#define X(c) {c, #c + 4},
        ACS_LIST
#undef X
    };
    int n = sizeof acs / sizeof acs[0];

    int col_w = 16;
    for (int i = 0; i < n; i++) {
        int row = i / 2, col = i % 2;
        if (row >= LINES - 2) break;
        if ((col + 1) * col_w >= COLS) break;
        mvaddch(row, col * col_w, acs[i].ch);
        mvprintw(row, col * col_w + 2, "%s", acs[i].name);
    }
    refresh();
    getch();
    endwin();
    return 0;
}

#include <unistd.h>

int main()
{
    write(1, "\033[2J", 5);   // clear screen
    write(1, "\033[0;0f", 7); // cursor reset
    write(1, "\33[0;10m\16h\17 BOARD\r\33[2d", 21);
    write(1, "\33[0;10m\16v\17 BTEE\33[2;17H", 22);
    write(1, "\33[0;10m\16~\17 BULLET\r\33[3d", 22);
    write(1, "\33[0;10m\16a\17 CKBOARD\33[17G", 23);
    write(1, "\33[0;10m\16.\17 DARROW\r\33[4d", 22);
    write(1, "\33[0;10m\16f\17 DEGREE\33[4;17H", 24);
    write(1, "\33[0;10m\16`\17 DIAMOND\r\33[5d", 23);
    write(1, "\33[0;10m\16q\17 HLINE\33[5;17H", 23);
    write(1, "\33[0;10m\16i\17 LANTERN\r\33[6d", 23);
    write(1, "\33[0;10m\16,\17 LARROW\33[6;17H", 24);
    write(1, "\33[0;10m\16m\17 LLCORNER\r\33[7d", 24);
    write(1, "\33[0;10m\16j\17 LRCORNER\33[17G", 24);
    write(1, "\33[0;10m\16t\17 LTEE\r\33[8d", 20);
    write(1, "\33[0;10m\16g\17 PLMINUS\33[17G", 23);
    write(1, "\33[0;10m\16n\17 PLUS\r\33[9d", 20);
    write(1, "\33[0;10m\16+\17 RARROW\33[9;17H", 24);
    write(1, "\33[0;10m\16u\17 RTEE\r\33[10d", 21);
    write(1, "\33[0;10m\16o\17 S1\33[10;17H", 21);
    write(1, "\33[0;10m\16s\17 S9\r\33[11d", 19);
    write(1, "\33[0;10m\16w\17 TTEE\33[11;17H", 23);
    write(1, "\33[0;10m\16-\17 UARROW\r\33[12d", 23);
    write(1, "\33[0;10m\16l\17 ULCORNER\33[17G", 24);
    write(1, "\33[0;10m\16k\17 URCORNER\r\33[13d", 25);
    write(1, "\33[0;10m\16x\17 VLINE\33[13;17H", 24);
    write(1, "\33[0;10m\16z\17 GEQUAL\r\33[14d", 23);
    write(1, "\33[0;10m\16y\17 LEQUAL\33[14;17H", 25);
    write(1, "\33[0;10m\16|\17 NEQUAL\r\33[15d", 23);
    write(1, "\33[0;10m\16{\17 PI\33[15;17H", 21);
    write(1, "\33[0;10m\16p\17 S3\r\33[16d", 19);
    write(1, "\33[0;10m\16r\17 S7\33[16;17H", 21);
    write(1, "\33[0;10m\16}\17 STERLING", 19);
    write(1, "\n", 1);
    return 0;
}

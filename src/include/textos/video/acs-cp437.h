// man 3 curs_addch - curs_addch(3X)
// https://www.ascii-codes.com/
static char cp437_acsget(char c)
{
    // clang-format off
    switch (c) {
    case '0':return 0xdb ; // solid square block
    case 'h':return 0xb0 ; // board of squares
    case 'v':return 0xc1; // bottom tee
    case '~':return 0xf9; // bullet
    case 'a':return 0xb1; // checker board (stipple)
    case '.':return 0x19; // arrow pointing down
    case 'f':return 0xf8; // degree symbol
    case '`':return 0x04; // diamond
    case '>':return 0xf2; // greater-than-or-equal-to
    case 'q':return 0xc4; // horizontal line
    case 'i':return 0x0f; // lantern symbol -> missing in vga
    case ',':return 0x1b; // arrow pointing left
    case 'y':return 0xf3; // less-than-or-equal-to
    case 'm':return 0xc0; // lower left-hand corner
    case 'j':return 0xd9; // lower right-hand corner
    case 't':return 0xc3; // left tee
    case '|':return 0x1c; // not-equal
    case '{':return 0xe3; // greek pi
    case 'g':return 0xf1; // plus/minus
    case 'n':return 0x2b; // plus
    case '+':return 0x1a; // arrow pointing right
    case 'u':return 0xb4; // right tee
    case 'o':return 0xc4; // scan line 1
    case 'p':return 0xc4; // scan line 3
    case 'r':return '-';  // scan line 7
    case 's':return '_';  // scan line 9
    case '}':return 0x9c; // pound-sterling symbol
    case 'w':return 0xc2; // top tee
    case '-':return 0x18; // arrow pointing up
    case 'l':return 0xda; // upper left-hand corner
    case 'k':return 0xbf; // upper right-hand corner
    case 'x':return 0xb3; // vertical line
    default :return ' ';
    }
    // clang-format on
}

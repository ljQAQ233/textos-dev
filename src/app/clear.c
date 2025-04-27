#include <stdio.h>

int main(int argc, char const *argv[])
{
    puts("\033[2J");   // clear screen
    puts("\033[0;0f"); // cursor reset
    return 0;
}

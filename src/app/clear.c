#include <stdio.h>

int main(int argc, char const *argv[])
{
    printf("\033[2J");   // clear screen
    printf("\033[0;0f"); // cursor reset
    fflush(stdout);
    return 0;
}

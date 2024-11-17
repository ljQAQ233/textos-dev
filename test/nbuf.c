// strace nbuf

#include <stdio.h>

int main()
{
    putchar('x');
    putchar('y');
    putchar('z');

    setbuf(stdout, NULL);
    putchar('x');
    putchar('y');
    putchar('z');

    return 0;
}

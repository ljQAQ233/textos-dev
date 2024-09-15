#include <stdio.h>

void test(unsigned x)
{
    printf("__builtin_ffs(%u) = %u\n", x, __builtin_ffs(x));
}

int main(int argc, char const *argv[])
{
    test(0);
    test(1);
    test(4);
    test(-1);
    return 0;
}

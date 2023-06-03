#include <stdio.h>
#include <stdarg.h>

void test_args (int n, ...)
{
    va_list args;
    va_start (args, n);
    
    int i = n;
    while (i--) {
        int v = va_arg (args, int);
        printf ("arg : %d\n",v);
    }
}

int main()
{
    test_args (11, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 233);
}

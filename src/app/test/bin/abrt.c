#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    printf("reachable\n");
    abort();
    printf("unreachable\n");
    return 0;
}

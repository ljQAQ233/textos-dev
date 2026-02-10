#include <stdlib.h>

int rand()
{
    return (int)random();
}

void srand(unsigned int seed)
{
    srandom(seed);
}

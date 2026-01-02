#include <stdlib.h>

int atoi(const char *str)
{
    return (int)strtol(str, NULL, 10);
}

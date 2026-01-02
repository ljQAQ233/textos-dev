#include <stdlib.h>

long atol(const char *str)
{
    return (long)strtol(str, NULL, 10);
}

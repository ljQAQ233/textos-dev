#include <stdlib.h>

long long atoll(const char *str)
{
    return (long long)strtoll(str, NULL, 10);
}

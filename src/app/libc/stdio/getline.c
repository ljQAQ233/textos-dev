#include "stdio.h"

/**
 * @see getdelim()
 */
ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *f)
{
    return getdelim(lineptr, n, '\n', f);
}
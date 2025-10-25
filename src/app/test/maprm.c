/*
 * test munmap()
 */
#include <stddef.h>
#include <sys/mman.h>

#define PS(x) ((x) * (1 << 12))
#define PROT  (PROT_READ | PROT_WRITE)

void *get(int n)
{
    char *m = mmap(NULL, PS(n), PROT, MAP_PRIVATE | MAP_ANON, -1, 0);
    for (int i = 0 ; i < PS(n) ; i += PS(1))
        m[i] = 'A';
    return m;
}

void put(void *v, int n)
{
    munmap(v, PS(n));
}

int main()
{
    // free all
    void *map1 = get(8);
    put(map1, 8);
    // free the left
    void *map2 = get(8);
    put(map2, 4);
    // free the right
    void *map3 = get(8);
    put(map3 + PS(4), 4);
    // free the middle
    void *map4 = get(8);
    put(map4 + PS(3), 3);
    return 0;
}

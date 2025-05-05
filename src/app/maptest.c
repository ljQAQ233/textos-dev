#include <app/api.h>
#include <stdio.h>

int main()
{
    char *s1 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    char *s2 = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE | MAP_ANON, -1, 0);
    s1[0] = 'h';
    printf("read : %c\n", s1[0]);
    s2[0] = 'h';
    printf("read : %c\n", s2[0]);
    return 0;
}
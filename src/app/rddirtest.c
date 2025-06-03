/*
 * a brief and rude test for `__seekdir`
 * print the entries of a dir reversely
 */
#include <app/api.h>
#include <stdio.h>
#include <assert.h>

char b[4096];

int main(int argc, char const *argv[])
{
    int fd = open(".", O_DIRECTORY);
    size_t pos = 0xffff;
    assert(__seekdir(fd, &pos) < 0);
    printf("dir entry num : %d\n", pos + 1);

    for (int i = pos ; i >= 0 ; i--) {
        size_t p = i;
        __seekdir(fd, &p);
        for (int sz = sizeof(dir_t) + 1, ret = 0 ; !ret ; sz++)
            ret = __readdir(fd, b, sz);
        dir_t *dir = (dir_t *)b;
        printf("[#%d] = %s\n", i, dir->name);
    }

    return 0;
}
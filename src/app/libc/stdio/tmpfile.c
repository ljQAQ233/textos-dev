#include "stdio.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char template[] = "/tmp/tmpfile_XXXXXX";

static int tmpfile_close(FILE *f)
{
    __stdio_close(f);
    unlink(f->tmppath);
    free(f->tmppath);
    return 0;
}

/*
 * The  tmpfile() function opens a unique temporary file in w+b mode. The file
 * will be automatically deleted when it is closed or the program terminates.
 */
FILE *tmpfile()
{
    char *path = strdup(template);
    if (!path) return 0;
    memcpy(path, template, sizeof(template));
    int fd = mkstemp(path);
    if (fd < 0) {
        free(path);
        return 0;
    }
    FILE *f = fdopen(fd, "w+b");
    if (!f) {
        close(fd);
        unlink(path);
        free(path);
        return 0;
    }
    f->close = tmpfile_close;
    f->tmppath = path;
    return f;
}

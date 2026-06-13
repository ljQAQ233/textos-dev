#include "stdio.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

static const char template[] = "/tmp/tmpfile_XXXXXX";

struct cookie
{
    char path[sizeof(template)];
};

static int tmpfile_close(FILE *f)
{
    struct cookie *c = f->cookie;
    __stdio_close(f);
    unlink(c->path);
    free(c);
    return 0;
}

/*
 * The  tmpfile() function opens a unique temporary file in w+b mode. The file
 * will be automatically deleted when it is closed or the program terminates.
 */
FILE *tmpfile()
{
    struct cookie *c = malloc(sizeof(struct cookie));
    if (!c) return 0;
    memcpy(c->path, template, sizeof(template));
    int fd = mkstemp(c->path);
    if (fd < 0) {
        free(c);
        return 0;
    }
    FILE *f = fdopen(fd, "w+b");
    if (!f) {
        close(fd);
        unlink(c->path);
        free(c);
        return 0;
    }
    f->close = tmpfile_close;
    f->cookie = c;
    return f;
}

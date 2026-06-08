#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int remove(const char *path)
{
    int ret = unlink(path);
    if (ret < 0 && errno == EISDIR) {
        ret = rmdir(path);
    }
    return ret;
}

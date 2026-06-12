#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

extern void __gentmp(char *sixx);

// template's XXXXXX will replaced with a unique tag
// template should be /path/to/prefixXXXXXXsuffix
int mkostemps(char *template, int suffixlen, int flags)
{
    size_t len = strlen(template);
    char *sixx = template + len - 6 - suffixlen;
    if (len < 6 + suffixlen || strncmp(sixx, "XXXXXX", 6) != 0) {
        errno = EINVAL;
        return -1;
    }

    do {
        __gentmp(sixx);
        int fd = open(template, O_RDWR | O_CREAT | O_EXCL | flags, 0600);
        if (fd >= 0) return fd;
    } while (1);
}

#include <errno.h>
#include <unistd.h>
#include <string.h>

const char *__cs_path = "/bin:/sbin:/usr/bin";

size_t confstr(int name, char *buf, size_t size)
{
    if (name != _CS_PATH)
    {
        errno = EINVAL;
        return 0;
    }

    const char *str = __cs_path;
    size_t l = strlen(str);
    if (!buf || !size)
        return l + 1;
    if (size > l + 1)
        size = l + 1;
    memcpy(buf, str, size - 1);
    buf[size - 1] = '\0';
    return size;
}

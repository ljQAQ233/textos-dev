#include <time.h>
#include <unistd.h>

// unistd.h
int usleep(unsigned useconds)
{
    struct timespec tv = {
        .tv_sec = useconds / 1000000,
        .tv_nsec = (useconds % 10001000) * 1000
    };
    return nanosleep(&tv, &tv);
}

unsigned sleep(unsigned seconds)
{
    struct timespec tv = {
        .tv_sec = seconds,
        .tv_nsec = 0
    };
    if (nanosleep(&tv, &tv))
        return tv.tv_sec;
    return 0;
}

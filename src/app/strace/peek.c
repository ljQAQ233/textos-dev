#include <errno.h>
#include <string.h>
#include <sys/ptrace.h>
#include <unistd.h>

#define MIN(a, b) (a > b ? b : a)

int peek_data(pid_t pid, unsigned long addr, void *buf, int len)
{
    size_t mask = sizeof(long) - 1;
    size_t rem = len;
    size_t off = addr & mask;
    size_t aligned = addr & ~mask;
    while (rem != 0) {
        union
        {
            long word;
            char bytes[sizeof(long)];
        } u;
        u.word = ptrace(PTRACE_PEEKDATA, pid, aligned, 0);
        if (u.word == -1 && errno) return -1;
        int cpysiz = MIN(rem, sizeof(long) - off);
        memcpy(buf, u.bytes + off, cpysiz);
        rem -= cpysiz;
        buf += cpysiz;
        aligned += sizeof(long);
        off = 0;
    }
    return 0;
}

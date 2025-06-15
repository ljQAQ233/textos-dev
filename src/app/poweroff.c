#include <sys/syscall.h>

int main(int argc, char const *argv[])
{
    syscall(SYS_poweroff);
    return -1;
}

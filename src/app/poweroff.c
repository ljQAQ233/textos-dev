#include <app/sys.h>
#include <app/api.h>

int main(int argc, char const *argv[])
{
    syscall(SYS_poweroff);
    return -1;
}

#include <textos/pwm.h>
#include <textos/syscall.h>

RETVAL(int) sys_poweroff()
{
    return poweroff();
}

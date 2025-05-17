#include <string.h>
#include <textos/errno.h>
#include <textos/utsname.h>
#include <textos/syscall.h>

utsname_t __kuname = {
	.sysname = UTS_SYSNAME,
	.nodename = UTS_NODENAME,
	.release = UTS_RELEASE,
	.version = UTS_VERSION,
	.machine = UTS_MACHINE,
};

__SYSCALL_DEFINE1(int, uname, utsname_t *, u)
{
	if (!u)
		return -EINVAL;
	memcpy(u, &__kuname, sizeof(utsname_t));
	return 0;
}

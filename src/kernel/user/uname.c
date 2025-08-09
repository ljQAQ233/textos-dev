#include <string.h>
#include <textos/task.h>
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

__SYSCALL_DEFINE2(int, sethostname, const char *, name, size_t, len)
{
	if (len > 64)
		return -EINVAL;
	if (task_current()->euid != 0)
		return -EPERM;
	memcpy(__kuname.nodename, name, len);
	return 0;
}

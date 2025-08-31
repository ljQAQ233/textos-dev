#ifndef	_SYS_MOUNT_H
#define	_SYS_MOUNT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

/* textos-specified */

int mount(const char *src, const char *dst);
int umount(const char *target);
int umount2(const char *target, int flags);

__END_DECLS

#endif

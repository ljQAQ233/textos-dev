#ifndef	_SYS_MOUNT_H
#define	_SYS_MOUNT_H

#include <sys/cdefs.h>

__BEGIN_DECLS

/* textos-specified */

int mount(char *src, char *dst);
int umount(char *target);
int umount2(char *target, int flags);

__END_DECLS

#endif
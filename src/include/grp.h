#ifndef	_GRP_H
#define	_GRP_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_gid_t
#define __NEED_size_t
#include <bits/alltypes.h>

struct group
{
    char *gr_name;   // group name 
    char *gr_passwd; // encrypted password
    gid_t gr_gid;    // group id
    char **gr_mem;   // null-terminated member list
};

struct group *getgrgid(gid_t __gid);
struct group *getgrnam(const char *__name);

int getgrgid_r(gid_t __gid, struct group *__grp, char *__buf, size_t __buflen, struct group **__result);
int getgrnam_r(const char *__name, struct group *__grp, char *__buf, size_t __buflen, struct group **__result);

__END_DECLS

#endif

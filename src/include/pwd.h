#ifndef _PWD_H
#define _PWD_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_size_t

#include <bits/alltypes.h>

struct passwd
{
    char *pw_name;   // user name
    char *pw_passwd; // encrypted password
    uid_t pw_uid;    // user id
    gid_t pw_gid;    // group id
    char *pw_gecos;  // optional user info
    char *pw_dir;    // path to home directory
    char *pw_shell;  // path to shell program
};

void setpwent();
void endpwent();
struct passwd *getpwent();

struct passwd *getpwuid(uid_t __uid);
struct passwd *getpwnam(const char *__name);

int getpwuid_r(uid_t __uid, struct passwd *__pwd, char *__buf, size_t __buflen, struct passwd **__result);
int getpwnam_r(const char *__name, struct passwd *__pwd, char *__buf, size_t __buflen, struct passwd **__result);

__END_DECLS

#endif
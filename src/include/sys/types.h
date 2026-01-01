#ifndef	_SYS_TYPES_H
#define	_SYS_TYPES_H

#define __NEED_key_t
#define __NEED_pid_t
#define __NEED_id_t
#define __NEED_uid_t
#define __NEED_gid_t
#define __NEED_mode_t
#define __NEED_nlink_t
#define __NEED_off_t
#define __NEED_ino_t
#define __NEED_dev_t
#define __NEED_blksize_t
#define __NEED_blkcnt_t
#define __NEED_fsblkcnt_t
#define __NEED_fsfilcnt_t

#define __NEED_time_t
#define __NEED_clock_t
#define __NEED_clockid_t
#define __NEED_suseconds_t
#define __NEED_useconds_t

#define __NEED_socklen_t
#define __NEED_sa_family_t

#define __NEED_ssize_t

#include <bits/alltypes.h>

// bash needs the compatibility
#if defined(_GNU_SOURCE) || defined(_BSD_SOURCE)
typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned u_int32_t;
typedef char *caddr_t;
typedef unsigned char u_char;
typedef unsigned short u_short, ushort;
typedef unsigned u_int, uint;
typedef unsigned long u_long, ulong;
typedef long long quad_t;
typedef unsigned long long u_quad_t;
#endif

#endif

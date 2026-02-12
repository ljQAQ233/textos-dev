#ifndef __TYPE_H__
#define __TYPE_H__

#define __NEED_int8_t
#define __NEED_int16_t
#define __NEED_int32_t
#define __NEED_int64_t

#define __NEED_uint8_t
#define __NEED_uint16_t
#define __NEED_uint32_t
#define __NEED_uint64_t

#define __NEED_intptr_t
#define __NEED_uintptr_t

#define __NEED_intmax_t
#define __NEED_uintmax_t

#define __NEED_char8
#define __NEED_char16

#define __NEED_size_t
#define __NEED_ssize_t

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
#define __NEED_struct_timeval
#define __NEED_struct_timespec

#define __NEED_socklen_t
#define __NEED_sa_family_t

#define __NEED_sigset_t

#define __NEED_struct_iovec
#define __NEED_struct_winsize

#include <bits/alltypes.h>

typedef int8_t   int8;
typedef int16_t  int16;
typedef int32_t  int32;
typedef int64_t  int64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef unsigned uint;

typedef int64_t  intn_t;
typedef uint64_t uintn_t;

typedef uint8_t   bool;
typedef uintptr_t addr_t;

typedef uint32_t blkno_t;

#define IOV_MAX 1024

typedef struct iovec iovec_t;

typedef signed int keysym_t;

#define aka(x) \
    struct x;  \
    typedef struct x x##_t

aka(devst);
aka(node);
aka(dirctx);
aka(superblk);
aka(buffer);
aka(vm_region);

#undef aka

#endif

#include "strace.h"
#include <sys/stat.h>
#include <sys/uio.h>

#define defst(N)       struct field _st_##N[] = {
#define f_dec(N)       {_STR(N), offsetof(S, N), reftyb_auto(nulstruct(S, N), )}
#define f_hex(N)       {_STR(N), offsetof(S, N), reftyb_auto(nulstruct(S, N), x)}
#define f_oct(N)       {_STR(N), offsetof(S, N), reftyb_auto(nulstruct(S, N), o)}
#define f_str(N)       {_STR(N), offsetof(S, N)}
#define f_bits(N)      {_STR(N), offsetof(S, N)}
#define f_struct(N, T) {_STR(N), offsetof(S, N), refst(T)}
#define f_custom(N)    {_STR(N), offsetof(S, N)}
#define endst()      \
    {                \
        .name = NULL \
    }                \
    }

#define refst(N) (_st_##N)

#define regst(N)                   \
    struct type _ty_##N = {        \
        .name = _STR(N),           \
        .size = 0,                 \
        .cls = CLASS_ST,           \
        .printer = st_printer,     \
        .ST = {.field = refst(N)}, \
    }

#define nulstruct(st, member) (((st *)0)->member)

#define S struct stat
defst(stat) //
    f_dec(st_dev),
    f_dec(st_ino), f_oct(st_mode), f_dec(st_nlink), f_dec(st_uid),
    f_dec(st_gid), f_dec(st_rdev), f_dec(st_size), f_dec(st_blksize),
    f_dec(st_blocks), endst();
regst(stat);
#undef S

#define S struct iovec
defst(iovec) //
    f_hex(iov_base),
    f_dec(iov_len), endst();
regst(iovec);
#undef S

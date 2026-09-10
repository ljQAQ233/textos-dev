#include "minix.h"
#define MINIX_VER    MINIX_V3
#define MAX_FILENAME 60
#define MAX_Z9IDX    10
#define SUPER_ONDISK V3_SUPER_ONDISK
#define MINIX_OP     __minix3_op
#define MINIX_INIT   __fs_init_minix3
#include "v2_inode.h"
#include "v3_super.h"
typedef u32 mino_t;
typedef u32 mzone_t;

#define FLEXIBLE_BLKSZ
#define FLEXIBLE_FIRST_DATA_ZONE
#define NATIVE_SYMLINK

#include "minix.c"

STATIC_ASSERT(offsetof(minix_super_t, magic) == V3_SUPER_MAGIC_OFFSET,
              "bad minix3 superblock::magic offset");

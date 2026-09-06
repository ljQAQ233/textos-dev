#include "v0_super.h"

// v3 super
typedef struct
{
    u32 inodes;            /* # usable inodes on the minor device */
    u16 nzones;            /* total device size, including bit maps etc */
    u16 imap_blocks;       /* # of blocks used by inode bit map */
    u16 zmap_blocks;       /* # of blocks used by zone bit map */
    u16 firstdatazone_old; /* number of first data zone (small) */
    u16 log_zone_size;     /* log2 of blocks/zone */
    u16 flags;             /* FS state flags */
    u32 max_size;          /* maximum file size on this device */
    u32 zones;             /* number of zones (replaces s_nzones in V2) */
    u16 magic;             /* magic number to recognize super-blocks */
    u16 pad2;              /* try to avoid compiler-dependent padding */
    u16 block_size;        /* block size in bytes. */
    u8 disk_version;       /* filesystem format sub-version */
    int v3_end[0];
    MINIX_V0_SUPER;
} minix_super_t;

#define V3_SUPER_ONDISK offsetof(minix_super_t, v3_end)

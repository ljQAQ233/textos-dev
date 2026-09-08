#pragma once

#define MINIX_V1    0x137f
#define MINIX_V1_30 0x138f
#define MINIX_V2    0x2468
#define MINIX_V2_30 0x2478
#define MINIX_V3    0x4d5a

#define V1_SUPER_MAGIC_OFFSET 16
#define V3_SUPER_MAGIC_OFFSET 24

extern superblk_t *__fs_init_minix1(devst_t *, buffer_t *);
extern superblk_t *__fs_init_minix3(devst_t *, buffer_t *);

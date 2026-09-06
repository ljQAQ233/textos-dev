#include "v0_super.h"

// v1 / v2 super
typedef struct minix_super
{
    u16 inodes;        // 节点总数
    u16 zones;         // 逻辑块总数
    u16 imap_blocks;   // inode 位图占用块数
    u16 zmap_blocks;   // 逻辑块位图占用块数
    u16 firstdatazone; // 第一个数据块编号
    u16 log_zone_size; // log2(每逻辑块包含的物理块数)
    u32 max_size;      // 单文件最大大小
    u16 magic;         // 文件系统魔数(0x137F)
    int v1_end[0];
    u16 pad;
    u32 zones32; // 用于替换 u16 的 zones
    int v2_end[0];
    MINIX_V0_SUPER;
} minix_super_t;

#define V1_SUPER_ONDISK offsetof(minix_super_t, v1_end)
#define V2_SUPER_ONDISK offsetof(minix_super_t, v2_end)

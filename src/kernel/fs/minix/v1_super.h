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
} minix_super_t;


typedef struct minix_inode
{
    u16 mode;    // 文件类型及权限
    u16 uid;     // 所有者用户ID
    u32 size;    // 文件大小(字节)
    u32 mtime;   // 修改时间(时间戳)
    u8 gid;      // 所有者组ID
    u8 nlinks;   // 硬链接数
    u16 zone[9]; // 数据块指针(0-6直接，7间接，8双间接 v1 不用)
} minix_inode_t;


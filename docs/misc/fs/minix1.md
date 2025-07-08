# minix fs v1

magic = 0x137f

- 初代 minix 文件系统
- 最初 linux 用的文件系统

# 6 个部分

minix1 文件系统分区 6 部分:

- 引导块
- 超级块
- inode 位图
- 逻辑块位图
- inode
- 数据区

---

目录项 保存在数据区

---

一个块 是 1KB

# 超级块

```c++
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
```

zone[x] 是相对于分区开头的块号, 为 0 未分配

# inode

```c++
typedef struct minix_inode
{
    u16 mode;          // 文件类型及权限
    u16 uid;           // 所有者用户ID
    u32 size;          // 文件大小(字节)
    u32 mtime;         // 修改时间(时间戳)
    u8 gid;            // 所有者组ID
    u8 nlinks;         // 硬链接数
    u16 zone[9];       // 数据块指针(0-6直接，7间接，8双间接 v1 不用)
} minix_inode_t;
```

# direct

```c++
typedef struct direct
{
    u16 ino;
    char name[14];
} minix_direct_t;
```

# 查找文件

root 的 inode number 是 1

`(ri = minix_iget(1))->zone` 存储了目录项 `struct direct[]`

目录项有 `ri->size` 项 (包含空闲)

有 `.` 与 `..`. 根目录是自己

在目录中去搜寻, 搜到了 inode number, 再去 iget 就可以了

# 创建文件

1. 分配 inode
2. 分配 zones (可选)

# 创建节点

- ok - `S_IFCHR`
- ok - `S_IFBLK`
- no - `S_IFIFO`

# 关于间接块

zone[7] -> u16[1024 / 2] ib (indirect block)

直接 + 间接块 -> 518 K

# minix fs magic overview

| 版本     | 文件系统名    | magic    | 说明                           |
| -------- | ------------- | -------- | ------------------------------ |
| Minix v1 | `MINIX_V1`    | `0x137F` | 原始 Minix，14 字节文件名      |
| Minix v1 | `MINIX_V1_30` | `0x138F` | Minix v1，30 字节文件名扩展    |
| Minix v2 | `MINIX_V2`    | `0x2468` | 支持 16/32 位 inode、大文件    |
| Minix v2 | `MINIX_V2_30` | `0x2478` | v2 + 30 字节文件名             |
| Minix v3 | `MINIX_V3`    | `0x4d5a` | 新 superblock 格式，支持更大卷 |

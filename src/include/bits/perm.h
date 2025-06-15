#ifndef S_IRUSR
// 特殊权限位 (特殊模式)
#define S_ISUID 04000  // 设置用户 ID 位(Set-user-ID)执行文件时切换为文件所有者的权限
#define S_ISGID 02000  // 设置组 ID 位(Set-group-ID)执行文件时切换为文件所属组的权限
#define S_ISVTX 01000  // 粘着位(Sticky bit)，通常用于目录中防止非所有者删除文件(如 /tmp)

// 所有者权限 (User)
#define S_IRUSR 0400   // 所有者可读(r)
#define S_IWUSR 0200   // 所有者可写(w)
#define S_IXUSR 0100   // 所有者可执行(x)
#define S_IRWXU 0700   // 所有者读/写/执行权限的组合(rwx)

// 群组权限 (Group)
#define S_IRGRP 0040   // 所属组可读(r)
#define S_IWGRP 0020   // 所属组可写(w)
#define S_IXGRP 0010   // 所属组可执行(x)
#define S_IRWXG 0070   // 所属组读/写/执行权限的组合(rwx)

// 其他用户权限 (Others)
#define S_IROTH 0004   // 其他用户可读(r)
#define S_IWOTH 0002   // 其他用户可写(w)
#define S_IXOTH 0001   // 其他用户可执行(x)
#define S_IRWXO 0007   // 其他用户读/写/执行权限的组合(rwx)
#endif
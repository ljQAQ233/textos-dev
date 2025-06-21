#include <stdio.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>

void dump(char *path, struct stat *sb)
{
    printf("  File: %s\n", path);
    printf("  Size: %-10lu Blocks: %-10ld IO Block: %-6ld\n",
        sb->st_size, sb->st_blocks, sb->st_blksize);
    printf("Device: %-3u,%-3u Inode: %-10lu Links: %-3lu\n",
        major(sb->st_dev), minor(sb->st_dev), sb->st_ino, sb->st_nlink);
    printf("  Mode: %07o\n", sb->st_mode);
    printf("Access: %lld\n", sb->st_atime);
    printf("Modify: %lld\n", sb->st_mtime);
    printf("Change: %lld\n", sb->st_ctime);
}

int main(int argc, char const *argv[])
{
    if (argc == 1)
        return 1;

    struct stat sb;
    for (int i = 1 ; i < argc ; i++)
    {
        char *path = (char *)argv[i];
        if (stat(path, &sb) < 0)
            perror("stat");
        else
            dump(path, &sb);
    }

    return 0;
}

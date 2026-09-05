// v2 inode
typedef struct
{
    u16 mode;     /* file type, protection, etc. */
    u16 nlinks;   /* how many links to this file */
    u16 uid;      /* user id of the file's owner */
    u16 gid;      /* group number */
    u32 size;     /* current file size in bytes */
    u32 atime;    /* time of last access (V2 only) */
    u32 mtime;    /* when was file data last changed */
    u32 ctime;    /* when was inode itself changed (V2 only)*/
    u32 zone[10]; /* zone numbers for direct, ind, and dbl ind */
} minix_inode_t;

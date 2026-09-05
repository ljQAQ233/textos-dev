# minix history - inode

## v1

```c
struct inode {
  u16_t i_mode;		/* file type, protection, etc. */
  u16_t i_uid;		/* user id of the file's owner */
  u32_t i_size;		/* current file size in bytes */
  i32_t i_modtime;	/* when was file data last changed */
  u8_t i_gid;		/* group number */
  u8_t i_nlinks;	/* how many links to this file */
  u16_t i_zone[9];	/* zone numbers for direct, ind, and dbl ind */
  ...
};
```

## v2

```c
struct inode {
  u16_t i_mode;     /* file type, protection, etc. */
  u8_t  i_nlinks;	/* how many links to this file */
  u16_t i_uid;		/* user id of the file's owner */
  u8_t  i_gid;		/* group number */
  u32_t i_size;		/* current file size in bytes */
  u32_t i_atime;	/* time of last access (V2 only) */
  u32_t i_mtime;	/* when was file data last changed */
  u32_t i_ctime;	/* when was inode itself changed (V2 only)*/
  u32_t i_zone[10]; /* zone numbers for direct, ind, and dbl ind */
  ...
};
```

## v3

```c
struct inode {
  u16_t i_mode;		/* file type, protection, etc. */
  u16_t i_nlinks	/* how many links to this file */
  u16_t i_uid;		/* user id of the file's owner */
  u16_t i_gid;		/* group number */
  u32_t i_size;		/* current file size in bytes */
  u32_t i_atime;	/* time of last access (V2 only) */
  u32_t i_mtime;	/* when was file data last changed */
  u32_t i_ctime;	/* when was inode itself changed (V2 only)*/
  u32_t i_zone[10]; /* zone numbers for direct, ind, and dbl ind */
  ...
};
```

# diff

意义不大, `v2` 对齐之后就是 `v3`, v1 与这两个都大相径庭

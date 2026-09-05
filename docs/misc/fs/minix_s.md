# minix history - super_block

## v1

> `http://download.minix3.org/previous-versions/Intel-1.1/floppy_disk6`

```c
struct super_block {
  u16_t s_ninodes;		/* # usable inodes on the minor device */
  u16_t s_nzones;		/* total device size, including bit maps etc */
  u16_t s_imap_blocks;	/* # of blocks used by inode bit map */
  u16_t s_zmap_blocks;	/* # of blocks used by zone bit map */
  u16_t s_firstdatazone;	/* number of first data zone */
  u16_t s_log_zone_size;	/* log2 of blocks/zone */
  u32_t s_max_size;		    /* maximum file size on this device */
  u16_t s_magic;			/* magic number to recognize super-blocks */
  ...
};
```

## v2

> `https://www.minix3.org/previous-versions/Intel-2.0.0/src/SYS.TAZ`

```c
struct super_block {
  u16_t s_ninodes;		/* # usable inodes on the minor device */
  u16_t s_nzones;		/* total device size, including bit maps etc */
  u16_t s_imap_blocks;		/* # of blocks used by inode bit map */
  u16_t s_zmap_blocks;		/* # of blocks used by zone bit map */
  u16_t s_firstdatazone;	/* number of first data zone */
  u16_t s_log_zone_size;	/* log2 of blocks/zone */
  u32_t s_max_size;		/* maximum file size on this device */
  u16_t s_magic;		/* magic number to recognize super-blocks */
  u16_t s_pad;			/* try to avoid compiler-dependent padding */
  u32_t s_zones;		/* number of zones (replaces s_nzones in V2) */
  ...
};
```

## v3

```c
struct super_block {
  u32_t s_ninodes;		/* # usable inodes on the minor device */
  u16_t s_nzones;		/* total device size, including bit maps etc */
  u16_t s_imap_blocks;	/* # of blocks used by inode bit map */
  u16_t s_zmap_blocks;	/* # of blocks used by zone bit map */
  u16_t s_firstdatazone_old;	/* number of first data zone (small) */
  u16_t s_log_zone_size;	/* log2 of blocks/zone */
  u16_t s_flags;	    /* FS state flags */
  u32_t s_max_size;		/* maximum file size on this device */
  u32_t s_zones;		/* number of zones (replaces s_nzones in V2) */
  u16_t s_magic;		/* magic number to recognize super-blocks */
  u16_t s_pad2;			/* try to avoid compiler-dependent padding */
  u16_t s_block_size;	/* block size in bytes. */
  u8_t  s_disk_version;	/* filesystem format sub-version */
  ...
};
```

# 源码篇

## v1

官方提供源码位于 `http://download.minix3.org/previous-versions/Intel-1.1`

从 `README` **可知** 文件系统源码位于 `floppy_disk6`

挂载即可:

```shell
sudo mount -o loop -t minix floppy_disk6 /mnt
```

当然你不介意的话 `vi` 打开也行 (/・・)ノ

# 相邻版本 diff

## v1 -> v2

```diff
@@ struct super_block @@
   u16_t s_log_zone_size;
   u32_t s_max_size;
   u16_t s_magic;
+  u16_t s_pad;
+  u32_t s_zones;
   ...
```

可以看到 v2 向下兼容 v1

## v2 -> v3

```diff
@@ struct super_block @@
-  u16_t s_ninodes;
+  u32_t s_ninodes;
   u16_t s_nzones;
   u16_t s_imap_blocks;
   u16_t s_zmap_blocks;
-  u16_t s_firstdatazone;
+  u16_t s_firstdatazone_old;
   u16_t s_log_zone_size;
-  u32_t s_max_size;
-  u16_t s_magic;
-  u16_t s_pad;
+  u16_t s_flags;
+  u32_t s_max_size;
   u32_t s_zones;
+  u16_t s_magic;
   ...
```

这个修改就比较大了, 这还是 **minix v2** 吗...

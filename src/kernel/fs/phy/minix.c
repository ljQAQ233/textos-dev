/**
 * @brief minix fs v1 implementation
 */
#include <textos/mm.h>
#include <textos/fs.h>
#include <textos/errno.h>
#include <textos/assert.h>
#include <textos/fs/inter.h>
#include <textos/dev/buffer.h>
#include <textos/klib/bitmap.h>
#include <textos/klib/string.h>

#define BLKSZ 1024

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

typedef struct direct
{
    uint16_t ino;
    char name[14];
} minix_direct_t;

#define MINIX_V1 0x137f

const uint zone_dib = 7;
const uint zone_ib1 = zone_dib + (BLKSZ / 2);
const uint zone_ib2 = zone_ib1 + (BLKSZ / 2) * (BLKSZ / 2);
const uint nodenr_perblk = BLKSZ / sizeof(minix_inode_t);
const uint xmapnr_perblk = BLKSZ * 8;
const uint direnr_perblk = BLKSZ / sizeof(minix_direct_t);

#define minix_boot()    (0)
#define minix_super()   (1)
#define minix_imap(sb)  (2)
#define minix_zmap(sb)  (2 + (sb)->imap_blocks)
#define minix_inode(sb) (2 + (sb)->imap_blocks + (sb)->zmap_blocks)

static minix_inode_t *minix_idup(minix_inode_t *mi)
{
    minix_inode_t *newi = malloc(sizeof(minix_inode_t));
    return memcpy(newi, mi, sizeof(minix_inode_t));
}

static buffer_t *minix_zget(superblk_t *sb, u16 zone[9], uint idx)
{
    /*
     * zone in `zone[]` 0 - 6 / 7 / 8 ?
     */
    if (idx < zone_dib)
    {
        if (!zone[idx])
            return NULL;
        return sb_bread(sb, zone[idx]);
    }
    if (idx < zone_ib1)
    {
        buffer_t *ib1_blk = sb_bread(sb, zone[zone_dib]);
        u16 *ib1 = ib1_blk->blk;
        u16 bi = ib1[idx - zone_dib];
        brelse(ib1_blk);
        if (!bi)
            return NULL;
        return sb_bread(sb, bi);
    }
    DEBUGK(K_WARN, "minix : blk idx out of range : %u\n", idx);
    return NULL;
}

static u16 minix_zalloc(superblk_t *sb);

/**
 * @brief allocate `ext` blocks and extend zone[]
 * 
 * @return int number of blockes not allocated
 */
static uint minix_zext(superblk_t *sb, u16 zone[9], uint ext)
{
    uint i = 0;
    for ( ; i < zone_dib && ext ; i++)
        if (zone[i] == 0)
            zone[i] = minix_zalloc(sb), ext--;
    if (!ext)
        return 0;

    buffer_t *ib1_blk;
    if (zone[zone_dib])
        ib1_blk = sb_bread(sb, zone[zone_dib]);
    else
    {
        zone[zone_dib] = minix_zalloc(sb);
        ib1_blk = sb_bread(sb, zone[zone_dib]);
        memset(ib1_blk->blk, 0, BLKSZ);
        bdirty(ib1_blk, true);
    }
    u16 *ib1 = ib1_blk->blk;
    for ( ; i < zone_ib1 && ext ; i++)
        if (ib1[i] == 0)
            ib1[i] = minix_zalloc(sb), ext--;
    bdirty(ib1_blk, true);
    brelse(ib1_blk);
    return ext;
}

static u16 minix_ialloc(superblk_t *sb)
{
    minix_super_t *msb = sb->sbi;
    for (uint i = 0 ; i < msb->inodes ; i += xmapnr_perblk)
    {
        uint bi = minix_imap(msb) + i / xmapnr_perblk;
        buffer_t *blk = sb_bread(sb, bi);
        bitmap_t bmp;
        bitmap_init(&bmp, blk->blk, xmapnr_perblk);
        size_t ino = bitmap_find(&bmp);
        if (ino != -1)
        {
            bitmap_set(&bmp, ino);
            bdirty(blk, true);
            brelse(blk);
            return (u16)(i + ino + 1);
        }
        brelse(blk);
    }
    return 0;
}

/*
 * zone number (blkno) is a offset from firstdatazone
 */
static u16 minix_zalloc(superblk_t *sb)
{
    minix_super_t *msb = sb->sbi;
    for (uint i = 0 ; i < msb->zones ; i += xmapnr_perblk)
    {
        uint bi = minix_zmap(msb) + i / xmapnr_perblk;
        buffer_t *blk = sb_bread(sb, bi);
        bitmap_t bmp;
        bitmap_init(&bmp, blk->blk, xmapnr_perblk);
        size_t blkno = bitmap_find(&bmp);
        if (blkno != -1)
        {
            bitmap_set(&bmp, blkno);
            bdirty(blk, true);
            brelse(blk);
            return (u16)(msb->firstdatazone + i + blkno);
        }
        brelse(blk);
    }
    return 0;
}

static void minix_ifree(superblk_t *sb, u16 ino)
{
    minix_super_t *msb = sb->sbi;
    uint bi = ino / xmapnr_perblk + minix_imap(msb);
    uint ii = ino % xmapnr_perblk;
    buffer_t *blk = sb_bread(sb, bi);
    bitmap_t bmp;
    bitmap_init(&bmp, blk->blk, xmapnr_perblk);
    bitmap_reset(&bmp, ii);
    bdirty(blk, true);
    brelse(blk);
}

static void minix_zfree(superblk_t *sb, u16 blkno)
{
    minix_super_t *msb = sb->sbi;
    blkno -= msb->firstdatazone;
    uint bi = blkno / xmapnr_perblk + minix_zmap(msb);
    uint ii = blkno % xmapnr_perblk;
    buffer_t *blk = sb_bread(sb, bi);
    bitmap_t bmp;
    bitmap_init(&bmp, blk->blk, xmapnr_perblk);
    bitmap_reset(&bmp, ii);
    bdirty(blk, true);
    brelse(blk);
}

static minix_inode_t *minix_iget(superblk_t *sb, u16 ino)
{
    if (ino == 0)
        return NULL;
    minix_super_t *msb = sb->sbi;
    if (ino > msb->inodes)
        return NULL;
    ino -= 1;
    uint bidx = ino / nodenr_perblk + minix_inode(msb);
    uint iidx = ino % nodenr_perblk;
    buffer_t *blk = sb_bread(sb, bidx);
    minix_inode_t *itab = blk->blk;
    minix_inode_t *newi = minix_idup(&itab[iidx]);
    brelse(blk);
    return newi;
}

static void minix_isync(superblk_t *sb, minix_inode_t *mi, u16 ino)
{
    minix_super_t *msb = sb->sbi;
    if (ino > msb->inodes)
        return;
    ino -= 1;
    uint bidx = ino / nodenr_perblk + minix_inode(msb);
    uint iidx = ino % nodenr_perblk;
    buffer_t *blk = sb_bread(sb, bidx);
    minix_inode_t *itab = blk->blk;
    memcpy(&itab[iidx], mi, sizeof(minix_inode_t));
    bdirty(blk, true);
    brelse(blk);
}

static inline dev_t minix_dev_parse(u16 dev)
{
    uint ma = dev >> 8;
    uint mi = dev & 0xff;
    return makedev(ma, mi);
}

static inline u16 minix_dev_make(dev_t dev)
{
    uint ma = major(dev);
    uint mi = minor(dev);
    if (ma > 0xff || mi > 0xff)
        DEBUGK(K_WARN, "minix: major(%u) or minor(%u) nr too large\n", ma, mi);
    return (ma << 8) | mi;
}

static u16 minix_lookup(node_t *dir, char *name)
{
    superblk_t *sb = dir->sb;
    minix_super_t *msb = sb->sbi;
    minix_inode_t *mdir = dir->pdata;
    uint entmax = mdir->size / sizeof(minix_direct_t);
    for (uint idx = 0 ; idx < entmax ; idx += sizeof(minix_direct_t))
    {
        uint zidx = idx / direnr_perblk;
        buffer_t *blk = minix_zget(sb, mdir->zone, zidx);
        minix_direct_t *ent = blk->blk;
        for (int eidx = 0 ; eidx < direnr_perblk ; eidx++)
        {
            minix_direct_t *ptr = &ent[eidx];
            if (strncmp(name, ptr->name, 14) == 0)
            {
                u16 ino = ptr->ino;
                brelse(blk);
                return ino;
            }
        }
        brelse(blk);
    }
    return 0;
}

#define align_up(x, y) ((y) * (((long)(x) + (y) - 1) / (y)))

/**
 * @brief edit entries. insert a directory entry or erase an entry
 * @param ino set as zero to do erasing
 */
static int minix_eddir(node_t *dir, char *name, u16 ino)
{
    superblk_t *sb = dir->sb;
    minix_super_t *msb = sb->sbi;
    minix_inode_t *mdir = dir->pdata;
    /*
     * align_up is not necessary, because the inner for-loop reads the entire block.
     * Even if mdir->size is not aligned to BLKSZ, as long as it's larger than a multiple
     * of BLKSZ, the loop will still cover all directory entries correctly.
     */
    uint entmax = mdir->size / sizeof(minix_direct_t);
    for (uint idx = 0 ; idx < entmax ; idx += direnr_perblk)
    {
        uint zidx = idx / direnr_perblk;
        buffer_t *blk = minix_zget(sb, mdir->zone, zidx);
        minix_direct_t *ent = blk->blk;
        for (int eidx = 0 ; eidx < direnr_perblk ; eidx++)
        {
            minix_direct_t *ptr = &ent[eidx];
            if (!ino && !strncmp(ptr->name, name, 14))
            {
                ptr->ino = 0;
                bdirty(blk, true);
                brelse(blk);
            }
            else if (ino && ptr->ino == 0)
            {
                ptr->ino = ino;
                strncpy(ptr->name, name, 14);
                bdirty(blk, true);
                brelse(blk);
                uint nsize = (idx * direnr_perblk + eidx + 1) * sizeof(minix_direct_t);
                if (mdir->size < nsize)
                {
                    mdir->size = nsize;
                    minix_isync(sb, mdir, dir->ino);
                }
                return 0;
            }
        }
        brelse(blk);
    }

    uint rem = minix_zext(sb, mdir->zone, 1);
    if (rem != 0)
        return -ENOSPC;
    return minix_eddir(dir, name, ino);
}

#define _(x) if (x) minix_zfree(sb, x); else break;

/**
 * @brief free data blocks
 */
static int minix_trunc(superblk_t *sb, u16 zone[9], uint zmax)
{
    uint zidx = 0;
    for ( ; zidx < MIN(zmax, zone_dib) ; zidx++);
    for ( ; zidx < zone_dib ; zidx++) _(zone[zidx]);

    if (zidx < zone_dib)
        return 0;
    ASSERTK(zone[zone_dib] != 0);

    buffer_t *ib1_blk = sb_bread(sb, zone[zone_dib]);
    u16 *ib1 = ib1_blk->blk;
    for ( ; zidx < MIN(zmax, zone_ib1) ; zidx++);
    for ( ; zidx < zone_ib1 ; zidx++)
        _(ib1[zidx - zone_dib]);
    minix_zfree(sb, zone[zone_dib]);

    return 0;
}

#undef _

static int minix_setupdir(superblk_t *sb, u16 zone[9], u16 ino, u16 pino)
{
    zone[0] = minix_zalloc(sb);
    if (!zone[0])
        return -ENOSPC;
    buffer_t *blk = minix_zget(sb, zone, 0);
    minix_direct_t *ent = blk->blk;

    ent[0].ino = ino;
    strcpy(ent[0].name, ".");
    ent[1].ino = pino;
    strcpy(ent[1].name, "..");
    
    bdirty(blk, true);
    brelse(blk);
    return 0;
}

static bool minix_isdev(mode_t mode)
{
    return (S_ISCHR(mode) || S_ISBLK(mode));
}

static node_t *minix_nodeget(superblk_t *sb, minix_inode_t *mi, u16 ino, char *name)
{
    node_t *node = malloc(sizeof(node_t));
    node->name = strdup(name);
    node->attr = 0;
    node->siz = mi->size;
    node->ino = ino;
    node->mode = mi->mode;
    node->atime = node->mtime = node->ctime = mi->mtime;
    node->dev = makedev(sb->dev->major, sb->dev->minor);
    node->rdev = minix_isdev(mi->mode) ? minix_dev_parse(mi->zone[0]) : NODEV;
    node->mount = NULL;
    node->pdata = mi;
    node->sb = sb;
    node->opts = sb->op;
    node->child = NULL;
    return node;
}

static int minix_namel(char *res, char *name)
{
    char *p = strchr(name, '/');
    size_t len = 0;
    if (!p)
        len = strlen(name);
    else
        len = p - name;
    if (len > 14)
        return -ENAMETOOLONG;
    strncpy(res, name, len);
    res[len] = 0;
    return 0;
}

static int minix_open(node_t *parent, char *name, u64 args, int mode, node_t **result)
{
    superblk_t *sb = parent->sb;
    minix_super_t *msb = sb->sbi;
    minix_inode_t *mdir = parent->pdata;

    char filname[15];
    if (minix_namel(filname, name) < 0)
        return -ENAMETOOLONG;

    u16 ino = minix_lookup(parent, filname);
    minix_inode_t *mi = minix_iget(sb, ino);
    if (!mi)
    {
        if (args & O_CREAT)
        {
            mode &= ~S_IFMT;
            ino = minix_ialloc(sb);
            if (ino == 0)
                goto nospace;
            if (minix_eddir(parent, filname, ino) < 0)
                goto nospace;
            mi = minix_iget(sb, ino);
            mi->mode = mode;
            mi->uid = 0;
            mi->size = 0;
            mi->mtime = arch_time_now();
            mi->gid = 0;
            mi->nlinks = 1;
            memset(mi->zone, 0, sizeof(mi->zone));
            if (args & O_DIRECTORY)
            {
                mi->size = 2 * sizeof(minix_direct_t);
                mi->mode |= S_IFDIR;
                if (minix_setupdir(sb, mi->zone, ino, parent->ino) < 0)
                    goto nospace;
            }
            else
                mi->mode |= S_IFREG;
            minix_isync(sb, mi, ino);
        }
        else
            return -ENOENT;
    }

    node_t *node = minix_nodeget(sb, mi, ino, filname);
    vfs_regst(node, parent);
    *result = node;
    return 0;

nospace:
    if (ino)
        minix_ifree(sb, ino);
    if (mi)
        free(mi);
    return -ENOSPC;
}

static int minix_mknod(node_t *parent, char *name, dev_t rdev, int mode, node_t **result)
{
    char filname[15];
    if (minix_namel(filname, name) < 0)
        return -ENAMETOOLONG;

    superblk_t *sb = parent->sb;
    minix_inode_t *mi = NULL;
    u16 ino = minix_ialloc(sb);
    if (ino == 0)
        goto nospace;
    if (minix_eddir(parent, filname, ino) < 0)
        goto nospace;
    mi = minix_iget(sb, ino);
    mi->mode = mode;
    mi->uid = 0;
    mi->size = 0;
    mi->mtime = arch_time_now();
    mi->gid = 0;
    mi->nlinks = 1;
    memset(mi->zone, 0, sizeof(mi->zone));
    mi->zone[0] = minix_dev_make(rdev);
    minix_isync(sb, mi, ino);

    node_t *node = minix_nodeget(sb, mi, ino, filname);
    vfs_regst(node, parent);
    *result = node;
    return 0;

nospace:
    if (ino)
        minix_ifree(sb, ino);
    if (mi)
        free(mi);
    return -ENOSPC;
}

static int minix_ioctl(node_t *this, int req, void *argp)
{
}

static int minix_chown(node_t *this, uid_t owner, gid_t group, bool ap)
{
    vfs_m_chown(this, owner, group, ap);
    superblk_t *sb = this->sb;
    minix_inode_t *mi = this->pdata;
    mi->uid = owner;
    mi->gid = group;
    minix_isync(sb, mi, this->ino);
    return 0;
}

static int minix_chmod(node_t *this, mode_t mode, bool clrsgid)
{
    /*
     * The mode passed to VFS has already been masked to exclude bits unsupported by Minix v1.
     */
    mode &= 0777;
    vfs_m_chmod(this, mode, clrsgid);
    superblk_t *sb = this->sb;
    minix_inode_t *mi = this->pdata;
    mi->mode &= ~0777;
    mi->mode |= mode;
    minix_isync(sb, mi, this->ino);
    return 0;
}

static int minix_close(node_t *this)
{
    return 0;
}

/*
 * `this` is not a directory
 */
static int minix_remove(node_t *this)
{
    superblk_t *sb = this->sb;
    minix_inode_t *mi = this->pdata;
    if (!minix_isdev(this->mode))
        minix_trunc(sb, mi->zone, 0);
    minix_eddir(this->parent, this->name, 0);
    minix_ifree(sb, this->ino);

    free(mi);
    return vfs_release(this);
}

static int minix_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    if (offset >= this->siz)
        return 0;

    superblk_t *sb = this->sb;
    minix_inode_t *mi = this->pdata;

    size_t rem = siz;
    size_t tot = 0;
    while (rem > 0 && offset < mi->size)
    {
        uint bidx = offset / BLKSZ;
        uint boff = offset % BLKSZ;
        buffer_t *blk = minix_zget(sb, mi->zone, bidx);
        if (!blk)
            break;

        size_t cpysiz = BLKSZ - boff;
        if (cpysiz > rem)
            cpysiz = rem;
        if (cpysiz > mi->size - offset)
            cpysiz = mi->size - offset;

        memcpy(buf, blk->blk + boff, cpysiz);
        brelse(blk);
        buf += cpysiz;
        offset += cpysiz;
        tot += cpysiz;
        rem -= cpysiz;
    }
    this->atime = arch_time_now();

    return tot;
}

static int minix_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    superblk_t *sb = this->sb;
    minix_inode_t *mi = this->pdata;

    size_t rem = siz;
    size_t tot = 0;
    char *src = buf;

    while (rem > 0)
    {
        uint bidx = offset / BLKSZ;
        uint boff = offset % BLKSZ;
        if (bidx >= zone_ib2)
            break;

        buffer_t *blk = minix_zget(sb, mi->zone, bidx);
        if (!blk)
        {
            uint err = minix_zext(sb, mi->zone, 1);
            if (err != 0)
                break;
            blk = minix_zget(sb, mi->zone, bidx);
        }

        size_t cpysiz = BLKSZ - boff;
        if (cpysiz > rem)
            cpysiz = rem;

        memcpy(blk->blk + boff, src, cpysiz);
        bdirty(blk, true);
        brelse(blk);

        src += cpysiz;
        offset += cpysiz;
        tot += cpysiz;
        rem -= cpysiz;
    }

    if (offset > mi->size)
    {
        this->siz = mi->size = offset;
        mi->mtime = arch_time_now();
        minix_isync(sb, mi, this->ino);
    }

    return tot > 0 ? (int)tot : -ENOSPC;
}

/*
 * sparse file
 *   - trunc - free data immediately
 *   - extend - record size only
 */
static int minix_truncate(node_t *this, size_t len)
{
    superblk_t *sb = this->sb;
    minix_inode_t *mi = this->pdata;
    uint zmax = DIV_ROUND_UP(len, BLKSZ);
    uint zhas = DIV_ROUND_UP(mi->size, len);
    if (zmax == zhas)
        return 0;
    else if (zmax < zhas)
        minix_trunc(sb, mi->zone, zmax);
    this->siz = mi->size = len;
    minix_isync(sb, mi, this->ino);
    this->mtime = arch_time_now();
    return 0;
}

static inline void init_ctx(dirctx_t *ctx, node_t *dir)
{
    ctx->sb = dir->sb;
    ctx->node = dir;
    ctx->pos = 0;
    ctx->bidx = 0;
    ctx->eidx = 0;
    ctx->stat = ctx_pre;
}

static int minix_dir_emit(dirctx_t *ctx, minix_direct_t *de, minix_inode_t *mi)
{
    uint len = strnlen(de->name, 14);
    uint type = dir_get_type(mi->mode);
    return dir_emit(ctx, de->name, len, de->ino, type);
}

static int minix_readdir(node_t *dir, dirctx_t *ctx)
{
    if (ctx->stat == ctx_inv)
        init_ctx(ctx, dir);
    if (ctx->stat == ctx_end)
        return EOF;
    dir->atime = arch_time_now();

    superblk_t *sb = dir->sb;
    minix_inode_t *mdir = dir->pdata;
    uint zmax = mdir->size / BLKSZ;
    uint zidx = ctx->pos / direnr_perblk;
    uint eidx = ctx->pos % direnr_perblk;
    for ( ; zidx <= zmax ; zidx++)
    {
        buffer_t *blk = minix_zget(sb, mdir->zone, zidx);
        minix_direct_t *ent = blk->blk;
        for ( ; eidx < direnr_perblk ; eidx++)
        {
            minix_direct_t *ptr = &ent[eidx];
            minix_inode_t *iptr = minix_iget(sb, ptr->ino);
            if (!ptr->ino)
                continue;
            if (!minix_dir_emit(ctx, ptr, iptr))
            {
                ctx->stat = ctx_end;
                brelse(blk);
                return 0;
            }
            ctx->pos++;
        }
        eidx = 0;
        brelse(blk);
    }

    return 0;
}

static int minix_seekdir(node_t *dir, dirctx_t *ctx, size_t *pos)
{
    if (ctx->stat == ctx_inv)
        init_ctx(ctx, dir);

    minix_inode_t *mdir = dir->pdata;
    uint entmax = mdir->size / sizeof(minix_direct_t);
    if (*pos < entmax)
    {
        ctx->pos = *pos;
        return 0;
    }

    ctx->pos = entmax - 1;
    return EOF;
}

static void *minix_mmap(node_t *this, vm_region_t *vm)
{
    return (void *)((addr_t)-EINVAL);
}

fs_opts_t __minix1_op;

superblk_t *__fs_init_minix(devst_t *dev)
{
    buffer_t *blk = bread(dev, BLKSZ, minix_super());
    minix_super_t *msb = malloc(sizeof(minix_super_t));
    memcpy(msb, blk->blk, sizeof(minix_super_t));
    brelse(blk);

    if (msb->magic != MINIX_V1)
        goto fail;

    superblk_t *sb = malloc(sizeof(superblk_t));
    sb->blksz = BLKSZ;
    sb->dev = dev;
    sb->root = NULL;
    sb->op = &__minix1_op;
    sb->sbi = msb;

    minix_inode_t *root = minix_iget(sb, 1);
    node_t *node = sb->root = minix_nodeget(sb, root, 1, "/");
    return sb;

fail:
    if (msb)
        free(msb);
    return NULL;
}

fs_opts_t __minix1_op = {
    minix_open,
    minix_mknod,
    minix_chown,
    minix_chmod,
    minix_remove,
    minix_readdir,
    minix_seekdir,
    minix_read,
    minix_write,
    minix_truncate,
    minix_mmap,
    minix_ioctl,
    minix_close,
};

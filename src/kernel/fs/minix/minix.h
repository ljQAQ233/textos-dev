#pragma once

typedef struct direct
{
    uint16_t ino;
    char name[14];
} minix_direct_t;

#define MINIX_V1 0x137f

#define z9idx_end_dire(sb) (7)
#define z9idx_end_ind1(sb) (z9idx_end_dire(sb) + (BLKSZ / 2))
#define z9idx_end_ind2(sb) (z9idx_end_ind1(sb) + (BLKSZ / 2) * (BLKSZ / 2))

#define z9idx_to_nlevels(z9idx)   ((z9idx) - 6)
#define nlevels_to_z9idx(nlevels) ((nlevels) + 6)

#define minix_boot()    (0)
#define minix_super()   (1)
#define minix_imap(sb)  (2)
#define minix_zmap(sb)  (2 + (sb)->imap_blocks)
#define minix_inode(sb) (2 + (sb)->imap_blocks + (sb)->zmap_blocks)

#define _minix_zidx_path(log_perlevel_entries, idx, level) \
    (idx >> ((level - 1) * log_perlevel_entries)) &        \
        ((1 << (log_perlevel_entries + 1)) - 1)

#define minix_zidx_path(sb, idx, level) _minix_zidx_path(9, idx, level)

#define MINIX_SBI(sb) ((minix_super_t *)((sb)->sbi))

#ifdef FLEXIBLE_BLKSZ
    #define BLKSZ MINIX_SBI(sb)->v0_block_size
#else
    #define BLKSZ 1024
#endif

#ifdef FLEXIBLE_PERBLK
    #define nodenr_perblk MINIX_SBI(sb)->v0_nodenr_perblk
    #define xmapnr_perblk MINIX_SBI(sb)->v0_xmapnr_perblk
    #define direnr_perblk MINIX_SBI(sb)->v0_direnr_perblk
#else
    #define nodenr_perblk (BLKSZ / sizeof(minix_inode_t))
    #define xmapnr_perblk (BLKSZ * 8)
    #define direnr_perblk (BLKSZ / sizeof(minix_direct_t))
#endif

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

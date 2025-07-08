#include <textos/fs.h>
#include <textos/mm.h>
#include <textos/errno.h>
#include <textos/fs/inter.h>
#include <textos/klib/time.h>
#include <textos/dev/buffer.h>
#include <textos/klib/string.h>

/*
 * fat32's mode fields are no longer valid after a shutdown, mode is held by vfs
 * to make sure elf to able to be executed, MODE_REG is set to "rwxr--r--"
 */
#define MODE_DIR 0755
#define MODE_REG 0744
#define EOC 0x0FFFFFFF
// end-of-cluster marker
#define is_eoc(val) (0x0FFFFFF8 <= val && val <= 0x0FFFFFFF) 

typedef struct _packed
{
    u8 jmp_bin[3];  // 跳转指令
    char oem_id[8]; // OEM 标识字符
    u16 sec_siz;    // 一个扇区字节数
    u8 sec_perclst; // 一个簇扇区数
    u16 rev_num;    // 保留扇区数
    u8 fat_num;     // FAT(File Allocation Tables) 的数量
    u16 ent_num;    // 根目录中条目最大数, 数量自动调整的 fat32 不需要, 为 0
    u16 sec_num16;  // 文件系统扇区总数
    u8 media;       // 媒体描述符
    u16 fat_siz16;  // FAT 的大小
    u16 sec_pertrk; // 一个磁道扇区数
    u16 head_num;   // 磁头数
    u32 hid_num;    // 隐藏扇区数, 即分区起始 lba
    u32 sec_num32;  // 文件系统扇区总数

    /* fat32 extended boot record */

    u32 fat_siz32;    // 一个 FAT 的大小
    u16 extflags;     //
    u16 fat_ver;      // 版本
    u32 root_clst;    // 根目录起始位置
    u16 info_sec;     // 存储文件系统信息的扇区 FSInfo
    u16 bkup_sec;     // 主引导扇区备份
    u8 rev1[12];      // 保留
    u8 drvnr;         // 标识介质类型, BIOS int 0x13  返回, 现代没什么用
    u8 rev2;          // 保留
    u8 extbootsig;    // 0x29
    u32 vol_id;       // 卷 id (序列号)
    char vol_lab[11]; // 卷标
    char type_str[8]; // "FAT32   "
    u8 bin[420];      // 引导代码
    u16 endsym;       // 0xAA55
} record_t;

STATIC_ASSERT(sizeof(record_t) == 512, "wrong size");

typedef struct _packed
{
    u32 lead_sig;
    u8 rev1[480];
    u32 struct_sig;
    u32 free_cnt;
    u32 free_next;
    u8 rev2[12];
    u32 trail_sig;
} fat_info_t;

#define INFO_LEAD_SIG   0x41615252
#define INFO_STRUCT_SIG 0x61417272
#define INFO_TRAIL_SIG  0xAA550000

typedef struct _packed
{
    u16 day   : 5;
    u16 month : 4;
    u16 year  : 7;  // 1980 - 2107
} fat_date_t;

typedef struct _packed
{
    u16 second : 5; // 0 - 29 -> 0s - 58s
    u16 minute : 6;
    u16 hour   : 5;
} fat_time_t;

STATIC_ASSERT(sizeof(fat_date_t) == 2, "wrong size");
STATIC_ASSERT(sizeof(fat_time_t) == 2, "wrong size");

typedef struct _packed {
    char name[11]; // base(8) + ext(3)
    u8 attr;
    u8 rev;
    u8 create_ms;
    fat_time_t create_tm;
    fat_date_t create_date;
    fat_date_t access_date;
    u16 clst_high;
    fat_time_t write_tm;
    fat_date_t write_date;
    u16 clst_low;
    u32 filesz;
} sentry_t;

#define LEN_NAME 8
#define LEN_EXT  3

typedef struct _packed
{
    u8 nr;
    u16 name1[5];
    u8 attr;
    u8 type;  // always zero
    u8 cksum_short;
    u16 name2[6];
    u16 clst; // always zero
    u16 name3[2];
} lentry_t;

#define _LEN_LONGW (13 * sizeof(u16))
#define _LEN_LONGC (13 * sizeof(u8))

STATIC_ASSERT(sizeof(sentry_t) == 32, "wrong size");
STATIC_ASSERT(sizeof(lentry_t) == 32, "wrong size");

#define FA_RO      0x01
#define FA_HIDDEN  0x02
#define FA_SYS     0x04
#define FA_VOLID   0x08
#define FA_DIR     0x10
#define FA_ARCHIVE 0x20
#define FA_LONG    (FA_RO | FA_HIDDEN | FA_SYS | FA_VOLID)

typedef struct
{
    superblk_t *sb;         // vfs super block
    unsigned sec_siz;       // the size of a sector
    unsigned fat_siz;       // the size of fat table
    unsigned clst_siz;      // the size of a cluster
    unsigned fat_entsz;     // the size of a fat entry
    unsigned fat_entpersec; // how many fat entries in a sector?
    unsigned fat_sec;       // fat table sector nr
    unsigned data_sec;      // an area after fat table
    unsigned root_sec;      // root sector number
    unsigned sec_perclst;   // how many sectors make up a cluster?
    unsigned fil_persec;    // how many file entries in a sector?
    unsigned fil_perclst;   // how many file entries in a cluster?
} fat_sbi_t;

static inline unsigned clst2sec(fat_sbi_t *sbi, unsigned clst)
{
    return (clst - 2) * sbi->sec_perclst + sbi->data_sec;
}

static inline unsigned sec2clst(fat_sbi_t *sbi, unsigned sec)
{
    return (sec - sbi->data_sec) / sbi->sec_perclst + 2;
}

static inline unsigned clst2fat(fat_sbi_t *sbi, unsigned clst)
{
    return sbi->fat_sec + (clst * sbi->fat_entsz) / sbi->sec_siz;
}

static inline unsigned get_next(fat_sbi_t *sbi, unsigned clst)
{
    unsigned idx = clst2fat(sbi, clst);
    buffer_t *blk = bread(sbi->sb->dev, sbi->sec_siz, idx);
    unsigned *tab = blk->blk;
    unsigned next = tab[clst % sbi->fat_entpersec];
    brelse(blk);
    return next;
}

static inline unsigned get_next_til(fat_sbi_t *sbi, unsigned from, unsigned delta)
{
    unsigned clst = from;
    while (delta-- > 0)
    {
        clst = get_next(sbi, clst);
    }
    return clst;
}

static inline sentry_t *entry_dup(sentry_t *e)
{
    sentry_t *n = malloc(sizeof(*n));
    return memcpy(n, e, sizeof(*n));
}

static inline bool is_free(sentry_t *e)
{
    u8 c = e->name[0];
    return c == 0xe5;
}

static inline bool is_none(sentry_t *e)
{
    u8 c = e->name[0];
    return c == 0;
}

static inline bool is_unused(sentry_t *e)
{
    u8 c = e->name[0];
    return c == 0xe5 || c == 0;
}

static inline void entry_erase(sentry_t *e)
{
    e->name[0] = 0xe5;
}

static inline u8 get_cksum(char *sent) 
{
    u8 sum = 0;
    u8 *ptr = (u8 *)sent;
    for (short len = 11 ; len != 0 ; len--) {
        // the operation is an unsigned char rotate right
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *ptr++;
    }
    return sum;
}

static unsigned alloc_clst(fat_sbi_t *sbi)
{
    for (unsigned curr = sec2clst(sbi, sbi->data_sec) ; ; )
    {
        buffer_t *blk = sb_bread(sbi->sb, clst2fat(sbi, curr));
        unsigned *tab = blk->blk;
        for (int i = curr % sbi->fat_entpersec ; i < sbi->fat_entpersec ; i++, curr++)
        {
            if (tab[i] == 0)
            {
                tab[i] = EOC;
                bdirty(blk, true);
                brelse(blk);
                return curr;
            }
        }
        brelse(blk);
    }
    return 0;
}

// lookup op

_UTIL_CMP();
_UTIL_NEXT();

#include <textos/klib/list.h>
#include <textos/klib/stack.h>

/**
 * @brief tells where the entry is
 */
typedef struct
{
    unsigned sec;  // sector number
    unsigned idx;  // entry index in this sec
    list_t link;   // links to lookup_t::link
} entlct_t;

#define lct_add(list, s, i)                        \
        do {                                          \
            entlct_t *l = malloc(sizeof(*l));         \
            l->sec = s;                               \
            l->idx = i;                               \
            list_insert_after((list), &l->link);      \
        } while (false);

#define lct_clr(list)                                  \
        do {                                           \
            entlct_t *l;                               \
            while (!list_empty(list)) {                \
                l = CR((list)->next, entlct_t, link);  \
                free(l);                               \
                list_remove((list)->next);             \
            }                                          \
        } while (false);                               \

typedef struct
{
    u32 pos;
    u32 clst;
    list_t link;
} lru_t;

#define CACHE_FIXED  32 // 跳表大小
#define CACHE_LRUMAX 16 // LRU 最大容量
#define CACHE_LRUGAP 1  // LRU 相邻记录的扇区的最小间隔

/*
 * 缓存: 跳表 + LRU
 * 跳表的粒度 根据访问的偏移量增加动态调整, 最开始是 clst_siz,
 * 一旦超过, 就会发生调整, 粒度翻倍, 每一次调整会将 表项 减半
 */
    
typedef struct
{
    u32 maxpos; // 当前最大偏移
    u32 unit;   // 当前 jmp 粒度
    u32 unitclst;
    u32 jmp[CACHE_FIXED];
    int lrucap; // lru 当前容量
    list_t lru; // lru 链表
} cache_t;

typedef struct
{
    fat_sbi_t *sbi;
    unsigned clst;
    node_t *node;
    stack_t ents; // entries
    list_t link;  // -> entlct_t.ents
    cache_t cache;
} lookup_t;

static inline void lru_record(lookup_t *lkp, u32 curpos, u32 curclst)
{
    lru_t *lru;
    cache_t *cache = &lkp->cache;

    // 间隔小于 CACHE_LRUCAP 不记录
    // if (!list_empty(&cache->lru))
    // {
    //     lru_t *prev = CR(cache->lru.next, lru_t, link);
    //     u32 prev_rela = prev->pos / lkp->f->clst_siz;
    //     u32 curr_rela = curpos / lkp->f->clst_siz;
    //     if (ABS((int64)curr_rela - (int64)prev_rela) < CACHE_LRUGAP)
    //         return ;
    // }

    if (cache->lrucap == CACHE_LRUMAX - 1)
    {
        list_t *list = list_popback(&cache->lru);
        lru = CR(list, lru_t, link);
        cache->lrucap--;
    }
    else
    {
        lru = malloc(sizeof(lru_t));
    }
    cache->lrucap++;
    lru->pos = curpos;
    lru->clst = curclst;
    list_pushhead(&cache->lru, &lru->link);
}

/* lru 被使用, 使它成为最新 */
static inline void lru_refresh(lookup_t *lkp, lru_t *lru)
{
    cache_t *cache = &lkp->cache;
    list_remove(&lru->link);
    list_pushhead(&cache->lru, &lru->link);
}

static inline void jmp_adjust(lookup_t *lkp, int *idx)
{
    cache_t *cache = &lkp->cache;
    while (*idx >= CACHE_FIXED)
    {
        cache->unit <<= 1;
        cache->unitclst <<= 1;
        for (int i = 0 ; i < (CACHE_FIXED >> 1) ; i++)
            cache->jmp[i] = cache->jmp[i << 1];
        for (int i = CACHE_FIXED >> 1 ; i < CACHE_FIXED ; i++)
            cache->jmp[i] = 0;
        *idx >>= 1;
    }
}

/*
 * 512K data testing:
 *  $ tr -dc 'a-zA-Z0-9' < /dev/urandom | fold -w 1023 | head -n 512 > src/resource/rand0
 *  >  (sh-xv6) > cp /rand0 /rand
 */
static u32 cache_search(lookup_t *lkp, u32 pos)
{
    // return get_next_til(lkp->f, lkp->clst, pos / 512);
    cache_t *cache = &lkp->cache;
    u32 pos_clst;
    u32 pos_rela = pos / lkp->sbi->clst_siz; // 相对文件开头的扇区数

    list_t *iter;
    u32 dist = -1;
    lru_t *srch = NULL;
    LIST_FOREACH(iter, &cache->lru)
    {
        lru_t *lru = CR(iter, lru_t, link);
        u32 lru_rela = lru->pos / lkp->sbi->clst_siz;
        if (lru_rela <= pos_rela)
        {
            if (dist > pos_rela - lru_rela)
            {
                dist = pos_rela - lru_rela;
                srch = lru;
            }
            if (dist == 0)
                break;
        }
    }

    // LRU 没找到再去找 跳表
    if (srch == NULL || dist > cache->unitclst)
    {
        int idx = pos_rela / cache->unitclst;
        jmp_adjust(lkp, &idx);
        while (idx > 0)
        {
            if (cache->jmp[idx])
                break;
            idx--;
        }

        u32 cur_clst = cache->jmp[idx];
        u32 cur_rela = idx * cache->unitclst;
        while (true)
        {
            // pos_rela 之后的区域已经被 unitclst 个扇区覆盖到了
            if (cur_rela + cache->unitclst >= pos_rela)
                break;
            idx++;
            jmp_adjust(lkp, &idx);
            cache->jmp[idx] = cur_clst = get_next_til(lkp->sbi, cur_clst, cache->unitclst);
            cur_rela += cache->unitclst;
        }
        dist = pos_rela - cur_rela;
        pos_clst = get_next_til(lkp->sbi, cur_clst, dist);
        lru_record(lkp, pos, pos_clst);
    }
    else
    {
        lru_refresh(lkp, srch);
        pos_clst = get_next_til(lkp->sbi, srch->clst, dist);
    }

    return pos_clst;
}

static void cache_init(lookup_t *lkp)
{
    cache_t *cache = &lkp->cache;
    cache->maxpos = 0;
    cache->unit = lkp->sbi->clst_siz;
    cache->unitclst = 1;
    memset(cache->jmp, 0, sizeof(cache->jmp));
    cache->jmp[0] = lkp->clst;
    cache->lrucap = 0;
    list_init(&cache->lru);
}

static inline ino_t inoget(lookup_t *lkp)
{
    /*
     * lkp may have no data clusters, but it must have a file entry
     * according to the doc Version 1.03, December 6, 2000, sector size must take on
     * values : 512, 1024, 2048 or 4096 if maximum compatibility with old implementations
     * is desired, only the value 512 should be used => 0 < index of dirent in a sector < 128
     */
    entlct_t *lct = CR(lkp->link.next, entlct_t, link);
    return ((u64)lct->sec << 7) | lct->idx;
}

static lookup_t *lkp_prt(lookup_t *lkp)
{
    node_t *n = lkp->node;
    if (n->sb->root == n)
        return lkp;
    return n->parent->pdata;
}

#include <textos/assert.h>

#define _(m, ptr)                                     \
    for (int i = 0; i < sizeof(m) / sizeof(*m); i++)  \
        if (m[i] == 0xFFFF)                           \
            break;                                    \
        else                                          \
            *ptr++ = m[i];                            \

static inline char *parse_namel(lentry_t *lent, char **buf)
{
    char *ptr = *buf;

    _(lent->name1, ptr);
    _(lent->name2, ptr);
    _(lent->name3, ptr);

    return *buf = ptr;
}

#undef _

static inline char *parse_name(sentry_t *sent, char *buf)
{
    char *ptr = buf;
    char *name = sent->name;

    for (int i = 0; i < LEN_NAME && name[i] != ' '; i++)
        *ptr++ = name[i];
    for (int i = 0; i < LEN_EXT && name[i + 8] != ' '; i++)
        *ptr++ = name[i + 8];

    *ptr = '\0';
    return buf;
}

#define _(m, ptr)                                      \
    for (int i = 0 ; i < sizeof(m) / sizeof(*m) ; i++) \
            m[i] = *ptr++;                             \

static inline char *make_namel(lentry_t *lent, char **buf)
{
    char *ptr = *buf;

    u16 tmp[13];
    for (int i = 0 ; i < 13 ; i++) {
        if (*ptr == 0) {
            tmp[i] = 0x0000;
            for (i = i + 1 ; i < 13 ; i++ )
                tmp[i] = 0xFFFF;
            break;
        }
        tmp[i] = *ptr++;
    }

    do {
        u16 *cpyr = tmp;
        _(lent->name1, cpyr);
        _(lent->name2, cpyr);
        _(lent->name3, cpyr);
    } while (false);

    return *buf = ptr;
}

#undef _

static const fat_time_t fat_time_null = { 0, 0, 0 };
static const fat_date_t fat_date_null = { 0, 0, 0 };

static inline time_t parse_time(fat_date_t fdate, fat_time_t ftime)
{
    rtc_tm_t rtctm = {
        .second = ftime.second << 1,
        .minute = ftime.minute,
        .hour = ftime.hour,
        .day = fdate.day,
        .month = fdate.month,
        .year = fdate.year + 1980,
    };
    return time_stamp(&rtctm);
}

static inline void make_time(time_t ts, fat_date_t *fdate, fat_time_t *ftime)
{
    rtc_tm_t tm;
    time_rtctm(ts, &tm);

    if (fdate)
    {
        fdate->day = tm.day;
        fdate->month = tm.month;
        fdate->year = tm.year - 1980;
    }

    if (ftime)
    {
        ftime->second = tm.second >> 1;
        ftime->minute = tm.minute;
        ftime->hour = tm.hour;
    }
}

static node_t *analyse_entry(stack_t *stk, unsigned *clst)
{
    ASSERTK(!stack_empty(stk));
    stacki_t *iter = stacki(stk, iter);
    sentry_t *main = stacki_data(iter);
                     stacki_next(iter);

    node_t *n = calloc(sizeof(*n));
    n->siz = main->filesz;
    n->attr = 0;
    if (main->attr & FA_DIR)
        n->mode |= S_IFDIR | MODE_DIR;
    else if (main->attr & FA_ARCHIVE)
        n->mode |= S_IFREG | MODE_REG;

    size_t cnt = stack_siz(stk) - 1;
    // only has a main entry (short entry)
    if (cnt == 0)
    {
        char buf[8 + 3];
        memset(buf, 0, sizeof(buf));
        n->name = strdup(parse_name(main, buf));
    }
    else
    {
        n->name = malloc(13 * 2 * cnt);
        
        do {
            char *ptr = n->name;
            while (cnt--)
            {
                parse_namel(stacki_data(iter), &ptr);
                            stacki_next(iter);
            }
        } while (false);
    }
    
    // location info
    if (main->clst_low || main->clst_high)
        *clst = main->clst_low | (u32)main->clst_high << 16;
    else
        *clst = 0;

    n->atime = parse_time(main->access_date, fat_time_null);
    n->mtime = parse_time(main->write_date, main->write_tm);
    n->ctime = n->mtime; // fat doesn't support ctime natively

    return n;
}

#define to_upper(c) ('a' <= c && c <= 'z' ? c - 32 : c)
#define to_lower(c) ('A' <= c && c <= 'Z' ? c + 32 : c)
#define align_up(x, y) ((y) * (((long)(x) + (y) - 1) / (y)))

/*
 * 生成 目录项 的栈
 * target 提供 name, time...
 * lkp 提供条目的定位信息
 */
static stack_t *make_entry(node_t *target, lookup_t *lkp, bool init)
{
    stack_t *stk = stack_init(NULL);
    stack_set(stk, free, NULL);
    ASSERTK(stk != NULL);
    
    bool longer = false;

    char *name = strdup(target->name);    
    /* 预处理 -> 去除末尾的 `.` */
    for (int i = strlen(name) - 1 ; i >= 0 ; i--) {
        if (name[i] == '.')
            name[i] = EOS;
        else
            break;
    }

    /* TODO: replace it -> 使用总名称长度来判断是完全错误的, 应该分开! */
    int len = strlen(name);
    if (len > 3 + 8)
    {
        longer = true;
        goto make;
    }

    // all the characters which are not supported by
    // short entry will be replaced by '_'
    size_t dotnr = 0;
    for (int i = 0 ; i < len ; i++) {
        switch (name[i]) {
            /* unsupported chars */
            case '+': case ',': case ';':
            case '=': case '[': case ']':
                name[i] = '_';
            
            case ' ':
            case 'a'...'z':
                longer = true;
                goto make;
            
            case '.':
                if (++dotnr > 1)
                    longer = true;
            break;

            default: break;
        }
    }
    
    //

    int ext_start, name_start;
    char _name[8];
    char _ext[3];
    int  _namei, _exti;
make:
    memset(_name, ' ', sizeof(_name));
    memset(_ext , ' ', sizeof(_ext ));

    // 对于 `.XXXX` 的 情况, `.XXXX` 不算做拓展名
    ext_start = 0;
    name_start = 0;
    for (int i = len - 1 ; i >= 0 ; i--) {
        if (name[i] == '.')
            ext_start = i+1;
    }
    for (int i = 0 ; i < len ; i++) {
        if (name[i] == '.')
            continue;

        name_start = i;
        break;
    }

    // 开始填充
    _exti = 0;
    _namei = 0;
    if (ext_start != 0) {
        for (int i = ext_start ; i < len ; i++) {
            char append = 0;
            switch (name[i]) {
                case ' ':
                    continue;

                default:
                    append = to_upper(name[i]);
                break;
            }

            if (_exti < 8)
                _ext[_exti++] = append;
        }
    } else {
        // 设置大一点, 满足 i < ext_start
        ext_start = 0xff;
    }

    for (int i = name_start ; i < len && i < ext_start ; i++) {
        char append = 0;
        switch (name[i]) {
            case '.':
            case ' ':
                continue;

            default:
                append = to_upper(name[i]);
            break;
        }

        if (_namei < 8)
            _name[_namei++] = append;
    }
    free(name);

    fat_sbi_t *f = target->sb->sbi;
    sentry_t *sent = calloc(sizeof(sentry_t));
    memcpy(sent->name, _name, 8);
    memcpy(sent->name + 8, _ext, 3);
    sent->filesz = target->siz;
    sent->attr = S_ISDIR(target->mode) ? FA_DIR : FA_ARCHIVE;

    if (lkp) {
        unsigned clst = lkp->clst;
        sent->clst_high = clst >> 16;
        sent->clst_low  = clst & 0xFFFF;
    }
    stack_push(stk, sent);

    // 接下来是长目录的主场 (有的话...)

    u8 cksum = get_cksum(sent->name);
    if (longer)
    {
        int cnt = DIV_ROUND_UP(len, 26);
        char *ptr = target->name;
        lentry_t *lent;
        for (int i = 0 ; i < cnt ; i++)
        {
            lent = calloc(sizeof(lentry_t));
            memset(lent->name1, 0xFF, sizeof(lent->name1));
            memset(lent->name2, 0xFF, sizeof(lent->name2));
            memset(lent->name3, 0xFF, sizeof(lent->name3));

            make_namel(lent, &ptr);

            // TODO : Check sum
            lent->attr = FA_LONG;
            lent->nr = i + 1;
            lent->cksum_short = cksum;
            
            stack_push(stk, lent);
        }

        /* set mask to mark the last entry */
        lent->nr |= 0x40;
    }

    if (init)
    {
        time_t now = arch_time_now();
        target->atime = now;
        target->mtime = now;
        target->ctime = now;
        make_time(now, &sent->create_date, &sent->create_tm);
    }
    make_time(target->atime, &sent->access_date, NULL);
    make_time(target->mtime, &sent->write_date, &sent->write_tm);
    return stk;
}

/*
    在一个父 entry 下查找子项, 并生成 lkp, 包含信息 (行为) 如下:
      - 一个栈
        - 保存着 entry 信息, 包含长短目录项
      - 一个 lct 定位子
        - 一个 描述当前 entry 所指向的 文件/目录 的簇号 (clst)
        - 一个 描述 entry 在文件系统中的具体位置的链表  (link)
      - vfs 抽象的子节点
          - 只是在物理文件系统层面的初始化 -> 不会参与 节点 有关虚拟文件系统部分的初始化
      - vfs 环境 (所处的文件系统信息) -> f
*/

static inline buffer_t *cl_bread(fat_sbi_t *sbi, unsigned clst, unsigned off)
{
    return sb_bread(sbi->sb, clst2sec(sbi, clst) + off);
}

static lookup_t *lookup_entry(lookup_t *prt, char *name)
{
    lookup_t *lkp = malloc(sizeof(*lkp));
    stack_init(&lkp->ents);
    stack_set(&lkp->ents, free, NULL);
    list_init(&lkp->link);

    fat_sbi_t *sbi = prt->sbi;
    bool wind = false;
    unsigned curr = prt->clst;
    while (true)
    {
        for (uint soff = 0 ; soff < sbi->sec_perclst ; soff++)
        { 
            buffer_t *blk = cl_bread(sbi, curr, soff);
            sentry_t *ents = blk->blk;

            for (int i = 0 ; i < sbi->fil_persec ; i++)
            {
                // name[0] == 0xe5 -> free entry
                if (is_free(&ents[i]))
                    goto ent_nxt;
                // name[0] == 0x00 -> end of dir
                if (is_none(&ents[i])) {
                    wind = true;
                    goto lkp_end;
                }

                lct_add(&lkp->link, blk->idx, i);
                if (ents[i].attr & FA_LONG)
                {
                    lentry_t *lent = (lentry_t *)&ents[i],
                                *prev = (lentry_t *)stack_top(&lkp->ents);
                    stack_push(&lkp->ents, entry_dup(&ents[i]));
                    if (prev && lent->cksum_short != prev->cksum_short)
                        DEBUGK(K_WARN, "cksum isn't unique\n");
                    goto ent_nxt;
                }

                if (!stack_empty(&lkp->ents))
                {
                    // the current one is short entry
                    u8 cksum = get_cksum(ents[i].name);
                    lentry_t *prev = stack_top(&lkp->ents);
                    if (prev->cksum_short != cksum)
                        DEBUGK(K_WARN, "cksum not matched with short ent\n");
                }
                
                // store the short entry so that we can handle
                // it and its attached ents at the same time
                stack_push(&lkp->ents, &ents[i]);

                // generate node
                lkp->node = analyse_entry(&lkp->ents, &lkp->clst);
                size_t cmpsiz = (size_t)(strchrnul(name, '/') - name);
                if (_cmp(name, lkp->node->name)) {
                    wind = true;
                    goto lkp_end;
                }

            lkp_rst:
                vfs_release(lkp->node);
                lkp->node = NULL;
                stack_clear(&lkp->ents);
                lct_clr(&lkp->link);

            ent_nxt:
                continue;
            lkp_end:
                break;
            }
            brelse(blk);
        }

        curr = get_next(sbi, curr);

        // reaches the end
        if (is_eoc(curr) || wind)
            break;
    }

    if (!lkp->node) {
        free(lkp);
        lkp = NULL;
    } else {
        lkp->sbi = sbi;
        lkp->node->ino = inoget(lkp);
        cache_init(lkp);
    }
    return lkp;
}

static inline void init_ctx(dirctx_t *ctx, node_t *dir);

static int lookup_skctx(dirctx_t *ctx, size_t *pos)
{
    if (ctx->pos == *pos) {
        return *pos;
    } else if (ctx->pos > *pos) {
        init_ctx(ctx, ctx->node);
        if (!*pos)
            return 0;
    }

    fat_sbi_t *sbi = ctx->sb->sbi;
    unsigned curr = sec2clst(sbi, ctx->bidx);
    unsigned eidx = ctx->eidx;
    unsigned soff = ctx->bidx % sbi->sec_perclst;
    bool wind = false;
    while (true)
    {
        for ( ; soff < sbi->sec_perclst ; soff++)
        {
            buffer_t *blk = cl_bread(sbi, curr, soff);
            sentry_t *ents = blk->blk;

            for ( ; eidx < sbi->fil_persec ; eidx++)
            {
                if (is_free(&ents[eidx]))
                    continue;
                if (is_none(&ents[eidx])) {
                    ctx->stat = ctx_inv;
                    goto lkp_end;
                }
                if (ents[eidx].attr & FA_LONG)
                    continue;
                else {
                    ctx->eidx = eidx+1;
                    ctx->bidx = blk->idx;
                    ctx->pos += 1;
                    if (ctx->pos == *pos)
                        goto lkp_end;
                }

                continue;

            lkp_end:
                wind = true;
                break;
            }
            brelse(blk);
            if (wind)
                goto done;
            eidx = 0;
        }
        soff = 0;

        curr = get_next(sbi, curr);
        if (is_eoc(curr))
        {
            ctx->stat = ctx_inv;
            goto done;
        }
    }
done:
    if (ctx->stat == ctx_pre)
        return 0;
    *pos = ctx->pos - 1;
    return EOF;
}

/*
 * ctx 记录上下文信息, ctx 描述 下一个将被读取的目录项
*/
static void lookup_byctx(dirctx_t *ctx)
{
    lookup_t *lkp = malloc(sizeof(*lkp));
    lkp->node = NULL;
    stack_init(&lkp->ents);
    stack_set(&lkp->ents, free, NULL);
    list_init(&lkp->link);

    fat_sbi_t *sbi = ctx->sb->sbi;
    bool wind = false;
    unsigned curr = sec2clst(sbi, ctx->bidx);
    unsigned eidx = ctx->eidx;
    unsigned soff = ctx->bidx % sbi->sec_perclst;
    while (true)
    {
        for ( ; soff < sbi->sec_perclst ; soff++)
        {
            buffer_t *blk = cl_bread(sbi, curr, soff);
            sentry_t *ents = blk->blk;

            for ( ; eidx < sbi->fil_persec ; eidx++)
            {
                if (is_free(&ents[eidx]))
                    continue;
                if (is_none(&ents[eidx]))
                    goto lkp_end;

                lct_add(&lkp->link, blk->idx, eidx);
                if (ents[eidx].attr & FA_LONG)
                {
                    lentry_t *lent = (lentry_t *)&ents[eidx],
                                *prev = (lentry_t *)stack_top(&lkp->ents);
                    stack_push(&lkp->ents, entry_dup(&ents[eidx]));
                    if (prev && lent->cksum_short != prev->cksum_short)
                        DEBUGK(K_WARN, "cksum isn't unique\n");
                    continue;
                }

                if (!stack_empty(&lkp->ents))
                {
                    // the current one is short entry
                    u8 cksum = get_cksum(ents[eidx].name);
                    lentry_t *prev = stack_top(&lkp->ents);
                    if (prev->cksum_short != cksum)
                        DEBUGK(K_WARN, "cksum not matched with short ent\n");
                }
                
                // store the short entry so that we can handle
                // it and its attached ents at the same time
                stack_push(&lkp->ents, &ents[eidx]);

                // generate node
                lkp->node = analyse_entry(&lkp->ents, &lkp->clst);
                lkp->node->ino = inoget(lkp);
                if (dir_emit_node(ctx, lkp->node))
                {
                    ctx->eidx = eidx+1;
                    ctx->bidx = blk->idx;
                    ctx->pos += 1;
                    vfs_release(lkp->node);
                    lkp->node = NULL;
                    stack_clear(&lkp->ents);
                    lct_clr(&lkp->link);
                    continue;
                }
            
            lkp_end:
                wind = true;
                break;
            }
            brelse(blk);
            if (wind)
                goto done;
            eidx = 0;
        }

        curr = get_next(sbi, curr);
        if (is_eoc(curr)) {
            ASSERTK(lkp->node == NULL);
            goto done;
        }
        soff = 0;
    }

done:
    free(lkp);
}

static size_t expand_to(fat_sbi_t *f, unsigned *start, size_t cnt, bool append);

/* 根据 stk 来申请, 结果保存到 lkp 的 list 中 */
static void lookup_alloc(lookup_t *prt, lookup_t *lkp, stack_t *stk)
{
    fat_sbi_t *sbi = prt->sbi;
    u32 curr = prt->clst;
    size_t start_idx = 0, curr_idx = 0;
    size_t cnt = stack_siz(stk);
    stack_init(&lkp->ents);
    list_init(&lkp->link);

    bool hit = false;
    while (true)
    {
        for (uint soff = 0 ; soff < sbi->sec_perclst ; soff++)
        {
            buffer_t *blk = cl_bread(sbi, curr, soff);
            sentry_t *ents = blk->blk;

            for (uint i = 0 ; i < sbi->fil_persec ; i++, curr_idx++)
            {
                if (!is_unused(&ents[i]))
                {
                    start_idx = curr_idx + 1;
                    lct_clr(&lkp->link);
                    continue;
                }

                lct_add(&lkp->link, blk->idx, i);
                if (curr_idx - start_idx + 1 == cnt)
                {
                    hit = true;
                    break;
                }
            }
            brelse(blk);
        }
        curr = get_next(sbi, curr);
        if (hit || is_eoc(curr))
            break;
    }
    
    if (!hit)
    {
        expand_to(sbi, &prt->clst, 1, true);
        lookup_alloc(prt, lkp, stk);
    }
}

// write a stack `update` with the location descrbed by the list in `ori`
static void lookup_save0(lookup_t *ori, stack_t *stk)
{
    ASSERTK(&ori->ents != stk);

    fat_sbi_t *sbi = ori->sbi;
    list_t *ptr;
    list_t *head = &ori->link;
    stacki_t *iter = stacki(stk, iter);

    /* 逆向遍历, 顺应 stack */
    LIST_FOREACH_REV(ptr, head)
    {
        entlct_t *lct = CR(ptr, entlct_t, link);
        buffer_t *blk = sb_bread(sbi->sb, lct->sec);
        sentry_t *ents = blk->blk;

        ASSERTK(lct->idx < sbi->fil_persec);
        void *entry =
            stacki_data(iter);
            stacki_next(iter);
        memcpy(&ents[lct->idx], entry, sizeof(sentry_t));

        bdirty(blk, true);
        brelse(blk);
    }

    stack_fini(&ori->ents);
    stack_move(stk, &ori->ents);
}

// 将 prt 目录下的项删除, 并替换为 chd 描述的
static void lookup_save1(lookup_t *prt, stack_t *chd)
{
    fat_sbi_t *sbi = prt->sbi;
    stacki_t *iter = stacki(chd, iter);
    for (unsigned curr = prt->clst ; ; )
    {
        for (uint soff = 0 ; soff < sbi->sec_perclst ; soff++)
        {
            buffer_t *blk = cl_bread(sbi, curr, soff);
            sentry_t *ents = blk->blk;

            for (uint i = 0 ; i < sbi->fil_persec ; i++)
            {
                if (!stacki_none(iter)) {
                    void *entry =
                        stacki_data(iter);
                        stacki_next(iter);
                    memcpy(&ents[i], entry, sizeof(sentry_t));
                } else {
                    memset(&ents[i], 0, sizeof(sentry_t));
                }
            }
            bdirty(blk, true);
            brelse(blk);
        }
        curr = get_next(sbi, curr);
        if (is_eoc(curr))
            break;
    }
    stack_fini(&prt->ents);
    stack_move(chd, &prt->ents);
}

/**
 * @brief erase entries of ori. ori->list holds a list of entries ori has.
 */
static void lookup_savex(lookup_t *ori)
{
    fat_sbi_t *sbi = ori->sbi;
    list_t *ptr;
    list_t *head = &ori->link;

    /* 逆向遍历, 顺应 stack */
    LIST_FOREACH_REV(ptr, head)
    {
        entlct_t *lct = CR(ptr, entlct_t, link);
        buffer_t *blk = sb_bread(sbi->sb, lct->sec);
        sentry_t *ents = blk->blk;
        ASSERTK(lct->idx < sbi->fil_persec);
        entry_erase(&ents[lct->idx]);
        bdirty(blk, true);
        brelse(blk);
    }
}

// 用栈更新目录项信息
static void lookup_update(lookup_t *lkp, stack_t *stk)
{
    ASSERTK(&lkp->ents != stk);

    if (stack_siz(&lkp->ents) == stack_siz(stk))
    {
        lookup_save0(lkp, stk);
    }
    else
    {
        lookup_savex(lkp);
        lookup_t *prt = lkp_prt(lkp);
        lookup_alloc(prt, lkp, stk);
        lookup_save0(lkp, stk);
    }
}

static inline unsigned fat_erase(unsigned *tabent, unsigned stat)
{
    unsigned tmp = *tabent;
    *tabent = stat;
    return tmp;
}

// cut 指定是否是截断操作.
// - true - 数据块
// - false - 数据块一个不剩
// cut == 0 仅当 start 为文件开头, 且释放所有数据块时
static void data_relse(fat_sbi_t *sbi, unsigned start, unsigned skip, bool cut)
{
    unsigned curr = start;
    unsigned stat = cut ? 0 : EOC;
    bool wind = false;
    while (!wind)
    {
        buffer_t *blk = sb_bread(sbi->sb, clst2fat(sbi, curr));
        unsigned *tab = blk->blk;
        if (skip)
            skip--;
        else
        {
            curr = fat_erase(&tab[curr % 128], stat);
            if (is_eoc(curr))
                wind = true;
        }
        bdirty(blk, true);
        brelse(blk);
    }
}

// 在原有的基础上拓宽/拓宽到 cnt 个扇区
static size_t expand_to(fat_sbi_t *sbi, unsigned *start, size_t cnt, bool append)
{
    if (*start == 0)
    {
        *start = alloc_clst(sbi);
        if (--cnt == 0)
            goto done;
    }

    ASSERTK(clst2sec(sbi, *start) >= sbi->data_sec);

    buffer_t *curblk;
    unsigned *icur, *iend;
    unsigned curr = *start, end;
    while (true)
    {
        curblk = sb_bread(sbi->sb, clst2fat(sbi, curr));
        icur = curblk->blk;

        u32 next = icur[curr % sbi->fat_entpersec];
        if (is_eoc(next))
            break;
        else
            curr = next;
        
        brelse(curblk);
        if (!append)
            if (--cnt == 0)
                goto done;
    }
    end = curr;
    iend = icur;

    for ( ; ; curr++)
    {
        // next sector of FAT1
        if (curr % sbi->fat_entpersec == 0)
        {
            bdirty(curblk, true);
            brelse(curblk);
            curblk = sb_bread(sbi->sb, clst2fat(sbi, curr));
            icur = curblk->blk;
        }

        if (!icur[curr % sbi->fat_entpersec])
        {
            /*
             * FAT[x] is 0 => cluster x is free
             * zero the free cluster
             */
            for (uint soff = 0 ; soff < sbi->sec_perclst ; soff++)
            {
                buffer_t *zero = cl_bread(sbi, curr, soff);
                memset(zero->blk, 0, sbi->sec_siz);
                bdirty(zero, true);
                brelse(zero);
            }

            iend[end % sbi->fat_entpersec] = curr;
            if (iend != icur)
            {
                // 这个空闲项已经作为一个节点链接到上一个索引,
                // 也就是上一个结尾, 此时的结尾应该被更新
                iend = icur;
            }
            end = curr;
            
            cnt -= 1;
        }

        if (cnt == 0)
        {
            iend[end % sbi->fat_entpersec] = EOC;
            bdirty(curblk, true);
            brelse(curblk);
            break;
        }
    }

done:
    return cnt;
}

static size_t node_expand(node_t *this, size_t siz)
{
    fat_sbi_t *sbi = this->sb->sbi;
    if (align_up(this->siz, sbi->clst_siz) >= siz)
        return siz;

    lookup_t *lkp = this->pdata;
    size_t cnt = DIV_ROUND_UP(siz, sbi->clst_siz);
    size_t res = expand_to(sbi, &lkp->clst, cnt, false);
    ASSERTK(res == 0);

    return siz;
}

static void node_update(node_t *this)
{
    lookup_t *lkp = this->pdata;
    stack_t *stk = make_entry(this, lkp, 0);
    lookup_update(lkp, stk);
}

static int byte_rd(node_t *n, void *buf, size_t rdsiz, size_t off)
{
    fat_sbi_t *sbi = n->sb->sbi;
    if (rdsiz == 0)
        return 0;
    if (off >= n->siz)
        return EOF;

    lookup_t *lkp = n->pdata;
    unsigned stsz = sbi->sec_siz;
    unsigned curr = cache_search(lkp, off);
    unsigned soff = (off / stsz) % sbi->sec_perclst;
    unsigned skip_byte = off % stsz;
    unsigned real = MIN(n->siz, off + rdsiz) - off;
    unsigned rem = real;
    while (true)
    {
        for ( ; soff < sbi->sec_perclst ; soff++)
        {
            //TODO
            buffer_t *tmp = cl_bread(sbi, curr, soff);
            size_t cpysiz = MIN(rem, skip_byte != 0 ? stsz - skip_byte : MIN(rem, stsz));
            memcpy(buf, tmp->blk + skip_byte, cpysiz);
            brelse(tmp);
            skip_byte = 0;

            buf += cpysiz;
            off += cpysiz;
            rem -= cpysiz;
            if (!rem)
                goto done;
        }
        curr = get_next(sbi, curr);
        if (is_eoc(curr))
            break;
        soff = 0;
    }

    // if (!is_eoc(curr))
    //     lru_record(lkp, off, curr);

    n->atime = arch_time_now();

done:
    return real;
}

static int byte_wr(node_t *n, void *buf, size_t wrsiz, size_t off)
{
    fat_sbi_t *sbi = n->sb->sbi;
    lookup_t *lkp = n->pdata;
    unsigned stsz = sbi->sec_siz;
    unsigned curr = cache_search(lkp, off);
    unsigned soff = (off / stsz) % sbi->sec_perclst;
    unsigned skip_byte = off % stsz;
    unsigned rem = wrsiz;
    while (true)
    {
        for ( ; soff < sbi->sec_perclst ; soff++)
        {
            buffer_t *ori = cl_bread(sbi, curr, soff);
            size_t cpysiz = MIN(rem, skip_byte != 0 ? stsz - skip_byte : MIN(rem, stsz));
            memcpy(ori->blk + skip_byte, buf, cpysiz);
            bdirty(ori, true);
            brelse(ori);
            skip_byte = 0;

            buf += cpysiz;
            rem -= cpysiz;
            if (!rem)
                goto done;
        }
        curr = get_next(sbi, curr);
        if (is_eoc(curr))
            break;
        soff = 0;
    }

    n->atime = arch_time_now();
    n->mtime = arch_time_now();
    n->ctime = n->mtime;

done:
    return wrsiz;
}

static sentry_t dirhead_1 = { ".          ", FA_DIR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static sentry_t dirhead_2 = { "..         ", FA_DIR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static node_t *create(node_t *prt, char *name, int mode)
{
    node_t *chd = calloc(sizeof(*chd));
    lookup_t *lkp = malloc(sizeof(*lkp));
    lookup_t *lkpp = prt->pdata;

    chd->name = strdup(name);
    chd->mode = mode;
    chd->siz = 0;
    chd->pdata = lkp;

    chd->dev = prt->dev;
    chd->sb = prt->sb;
    chd->opts = prt->opts;
    vfs_regst(chd, prt);

    unsigned clst = 0;
    if (S_ISDIR(chd->mode))
        clst = alloc_clst(chd->sb->sbi);

    lkp->sbi = prt->sb->sbi;
    lkp->clst = clst;
    lkp->node = chd;
    list_init(&lkp->link);
    stack_init(&lkp->ents);

    stack_t *stk = make_entry(chd, lkp, 1);
    lookup_alloc(lkpp, lkp, stk);
    lookup_save0(lkp, stk);

    // 目录开头需要一些预设 entry, 比如 `.` `..`
    if (S_ISDIR(chd->mode))
    {
        stack_t *init = stack_init(NULL);
        stack_push(init, &dirhead_2);
        stack_push(init, &dirhead_1);
        lookup_save1(lkp, init);
        stack_fini(init);
    }

    chd->ino = inoget(lkp);
    return chd;
}

static node_t *_fat32_open(node_t *dir, char *path)
{
    node_t *chd = NULL;
    lookup_t *lkp;
    if ((lkp = lookup_entry(dir->pdata, path)))
    {
        chd = lkp->node;
        chd->pdata = lkp;

        chd->opts = dir->opts;
        chd->dev = dir->dev;
        chd->rdev = NODEV;
        chd->sb = dir->sb;
        vfs_regst(chd, dir);
    }

    return chd;
}

static int fat32_open(node_t *parent, char *path, u64 args, int mode, node_t **result)
{
    int ret = 0;
    node_t *opened;

    opened = _fat32_open(parent, path);
    if (opened)
        goto end;

    if (args & O_CREAT)
    {
        /*
         * 按照习惯, 默认只能在一个已经存在的目录下创建新的节点,
         * 但是如果 Path 不到头, 就说明此文件的父目录也不存在, 所以直接返回 NULL
         */
        if (*_next(path) != 0) {
            ret = -ENOENT;
            goto end;
        }

        mode &= ~S_IFMT;
        if (args & O_DIRECTORY)
            mode |= S_IFDIR;
        else
            mode |= S_IFREG;
        opened = create(parent, path, mode);
    }
    else
    {
        ret = -ENOENT;
    }

end:
    if (ret < 0)
        opened = NULL;
    *result = opened;
    return ret;
}

/*
 * fat does not support device node. here we create an empty file and bind
 * it with a device only in memory (vfs node_t::rdev).
 */
static int fat32_mknod(node_t *parent, char *name, dev_t rdev, int mode, node_t **result)
{
#if !CONFIG_FAT_SIMU_DEV
    return -ENOSYS;
#endif
    node_t *node;
    int ret = fat32_open(parent, name, O_CREAT, MODE_REG, &node);
    if (ret < 0)
        return ret;
    node->mode = mode;
    node->rdev = rdev;
    *result = node;
    return 0;
}

static int fat32_close(node_t *this)
{
    lookup_t *lkp = this->pdata;
    node_update(this);
    vfs_release(this);
    lct_clr(&lkp->link);
    free(lkp);
    return 0;
}

static int fat32_remove(node_t *this)
{
    lookup_t *prt = this->parent->pdata;
    lookup_t *lkp = lookup_entry(prt, this->name);
    lookup_savex(lkp);
    // 释放数据区
    data_relse(this->sb->sbi, lkp->clst, 0, false);
    return fat32_close(this);
}

static int fat32_read(node_t *this, void *buf, size_t siz, size_t offset)
{
    return byte_rd(this, buf, siz, offset);
}

static int fat32_write(node_t *this, void *buf, size_t siz, size_t offset)
{
    bool chg = false;
    bool zero = !this->siz;
    if (siz + offset >= this->siz) {
        this->siz = node_expand(this, siz + offset);
        if (zero)
            cache_init(this->pdata);
        chg = true;
    }

    int ret = byte_wr(this, buf, siz, offset);
    if (chg)
        node_update(this);
    return ret;
}

static int fat32_truncate(node_t *this, size_t len)
{
    if (len == this->siz)
        return 0;

    if (len > this->siz)
        this->siz = node_expand(this, len);
    else
    {
        fat_sbi_t *sbi = this->sb->sbi;
        /* 当 Offset == 0 时, 释放掉所有簇 */
        bool all = (bool)(len == 0);
        lookup_t *lkp = this->pdata;
        unsigned clst = lkp->clst;
        unsigned skip = DIV_ROUND_UP(len, sbi->clst_siz);
        data_relse(sbi, clst, skip, all);
        this->siz = len;
    }
    node_update(this);

    return 0;
}

/*
 * for fat:
 *  - bidx is the sector number
 *  - eidx is the entry's offset from the sector start
 */
static inline void init_ctx(dirctx_t *ctx, node_t *dir)
{
    lookup_t *lkp = dir->pdata;
    ctx->sb = dir->sb;
    ctx->node = dir;
    ctx->pos = 0;
    ctx->bidx = clst2sec(ctx->sb->sbi, lkp->clst);
    ctx->eidx = 0;
    ctx->stat = ctx_pre;
}

static int fat32_readdir(node_t *dir, dirctx_t *ctx)
{
    if (ctx->stat == ctx_inv)
        init_ctx(ctx, dir);
    if (ctx->stat == ctx_end)
        return EOF;
    lookup_byctx(ctx);
    dir->atime = arch_time_now();
    return 0;
}

/**
 * @brief 调整 ctx 到 pos 位置

 * @retval 0   操作成功
 * @retval EOF 到达目录末端, pos 被设置成 目录最后一项的位置
 */
static int fat32_seekdir(node_t *dir, dirctx_t *ctx, size_t *pos)
{
    if (ctx->stat == ctx_inv)
        init_ctx(ctx, dir);
    return lookup_skctx(ctx, pos);
}

fs_opts_t __fat32_opts;

superblk_t *__fs_init_fat32(devst_t *dev)
{
    buffer_t *recblk = bread(dev, 512, 0);
    record_t *rec = recblk->blk;

    if (rec->endsym != 0xAA55)
        goto fail;

    if (rec->sec_siz != 512)
        goto fail;

    if (rec->ent_num != 0)
        goto fail;

    /*
     * a cluster is regarded as a data block unit.
     */
    unsigned fat_siz = rec->fat_siz32;
    unsigned fat_sec = rec->rev_num;
    unsigned data_sec = fat_sec + rec->fat_num * fat_siz;
    unsigned root_sec = (rec->root_clst - 2) * rec->sec_perclst + data_sec;
    DEBUGK(K_INIT,
           "fat32 -> tab : %#x (%u,%u) , first_data_sec : %#x , root_sec : %#x "
           "{%d}\n",
           fat_sec, rec->fat_num, fat_siz, data_sec, root_sec,
           rec->sec_perclst);

    superblk_t *sb = malloc(sizeof(*sb));
    fat_sbi_t *sbi = malloc(sizeof(*sbi));
    node_t *node = malloc(sizeof(*node));
    lookup_t *root = malloc(sizeof(*root));
    sb->blksz = rec->sec_siz;
    sb->dev = dev;
    sb->root = node;
    sb->op = &__fat32_opts;
    sb->sbi = sbi;

    sbi->sb = sb;
    sbi->sec_siz = rec->sec_siz;
    sbi->fat_siz = fat_siz;
    sbi->clst_siz = rec->sec_perclst * rec->sec_siz;
    sbi->fat_entsz = 4;
    sbi->fat_entpersec = sbi->sec_siz / sbi->fat_entsz;
    sbi->fat_sec = fat_sec;
    sbi->data_sec = data_sec;
    sbi->root_sec = root_sec;
    sbi->sec_perclst = rec->sec_perclst;
    sbi->fil_persec = sbi->sec_siz / 32;
    sbi->fil_perclst = sbi->clst_siz / 32;

    root->sbi = sbi;
    root->node = node;
    root->clst = sec2clst(sbi, root_sec);

    node->name = "/";
    node->mode = S_IFDIR | MODE_DIR;
    node->atime = arch_time_now();
    node->mtime = arch_time_now();
    node->ctime = arch_time_now();
    node->ino = 1;
    node->siz = 0;
    node->parent = node;
    node->child = node->next = NULL;
    node->dev = makedev(dev->major, dev->minor);
    node->rdev = NODEV;
    node->pdata = root;
    node->mount = NULL;
    node->sb = sb;
    node->opts = &__fat32_opts;

    return sb;

fail:
    return NULL;
}

fs_opts_t __fat32_opts = {
    fat32_open,
    fat32_mknod,
    noopt,
    fat32_close,
    fat32_remove,
    fat32_read,
    fat32_write,
    fat32_truncate,
    fat32_readdir,
    fat32_seekdir,
    noopt,
};

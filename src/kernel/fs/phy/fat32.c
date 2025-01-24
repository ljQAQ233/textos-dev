// WARN: cluster mechanism is not supported
// only one sector per cluster is adaptable
// refactor is necessary

#include <textos/fs.h>
#include <textos/fs/inter.h>
#include <textos/mm.h>
#include <textos/dev.h>
#include <textos/errno.h>
#include <textos/klib/list.h>
#include <textos/klib/stack.h>
#include <textos/panic.h>
#include <textos/assert.h>
#include <textos/dev/buffer.h>

#define SECT_SIZ 512

#include <string.h>

#define EOC 0x0FFFFFFF
// end-of-cluster marker
#define IS_EOC(val) (0x0FFFFFF8 <= val && val <= 0x0FFFFFFF) 

/* Biso parameter block */
typedef struct _packed
{
    u8      jmpbin[3];        // Jump instruction
    char    oem_id[8];        // OEM identifier
    u16     sec_siz;          // The num of bytes per sector
    u8      sec_perclus;      // The num of clusters per sector
    u16     sec_reversed;     // The num of reserved sectors
    u8      fat_num;          // The num of File Allocation Tables, often is 2
    u16     roots;            // The num of ents in root, 0 for fat32!
    u16     sec_num16;           
    u8      desc_media;          
    u16     fat_siz16;           
    u16     sec_pertrack;        
    u16     head_num;         // The num of heads
    u32     sec_hidden;       // The num of the hidden sectors
    u32     sec_num32;           
    u32     fat_siz32;           
    u16     extflags;         // Extended flags
    u16     fat_ver;             
    u32     root_clus;        // The start cluster of root
    u16     info;             // The information of this file system
    u16     mbr_backup;       // The backup sector of MBR
    u8      reserved1[12];      
    u8      driver_num;           
    u8      reserved2;          
    u8      extbootsig;         
    u32     vol_id;              
    char    vol_label[11];       
    char    fat_type[8];         
    u8      bin[356];         // Boot code
    part_t  ptab[4];          // Partition table
    u16     endsym;           // 0xAA55
} record_t;

STATIC_ASSERT(sizeof(record_t) == 512, "wrong size");

typedef struct _packed {
    u32 lead_sig;
    u8  rev1[480];
    u32 struct_sig;
    u32 free_cnt;
    u32 free_next;
    u8  rev2[12];
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

typedef struct _packed
{
    char   name[11]; // Base(8) + Ext(3)
    u8     attr;
    u8     rev;
    u8     create_ms;
    u16    create_tm;
    u16    create_date;
    u16    access_date;
    u16    clus_high;
    u16    write_tm;
    u16    write_date;
    u16    clus_low;
    u32    filesz;
} sentry_t;

#define LEN_NAME 8
#define LEN_EXT  3

typedef struct _packed
{
    u8  nr;
    u16 name1[5];
    u8  attr;
    u8  type;    // Always zero
    u8  cksum_short;
    u16 name2[6];
    u16 clus;    // Always zero
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

typedef struct {
    dev_t *dev;
 
    u64 fat_sec;
    u64 first_data_sec;
    u32 free_cnt;
    u32 free_next;
    mbr_t *mbr;       // The master boot sector of this partition
    record_t *record; // The boot record of this Fat32 file system
    part_t *pentry;   // The partition entry in mbr
} sysinfo_t;

static int fat32_open (node_t *parent, char *path, u64 args, node_t **result);
static int fat32_close (node_t *this);
static int fat32_remove (node_t *this);
static int fat32_read (node_t *this, void *buf, size_t siz, size_t offset);
static int fat32_write (node_t *this, void *buf, size_t siz, size_t offset);
static int fat32_truncate (node_t *this, size_t offset);
static int fat32_readdir (node_t *this, node_t **res, size_t idx);

fs_opts_t __fat32_opts = {
    .open = fat32_open,
    .close = fat32_close,
    .remove = fat32_remove,
    .read = fat32_read,
    .write = fat32_write,
    .truncate = fat32_truncate,
    .readdir = fat32_readdir,
};

FS_INITIALIZER(__fs_init_fat32)
{
    record_t *record = malloc(sizeof(record_t));
    hd->bread (hd, pentry->relative, record, 1);

    if (record->endsym != 0xAA55)
        goto fail; // error
    if (record->sec_siz != SECT_SIZ)
        goto fail; // unsupported
    
    u32 free_cnt, free_next;
    fat_info_t *fat_info = malloc(sizeof(fat_info_t));
    hd->bread (hd, record->info, fat_info, 1);
    if (fat_info->lead_sig == INFO_LEAD_SIG && fat_info->struct_sig == INFO_STRUCT_SIG && fat_info->trail_sig == INFO_TRAIL_SIG)
    {
        free_cnt = fat_info->free_cnt;
        free_next = fat_info->free_next;
    }
    else
    {
        free_next = 0;
    }

    u64 fat_siz = record->fat_siz16 == 0 ? record->fat_siz32 : record->fat_siz16;
    u64 fat_tab_sec = pentry->relative + record->sec_reversed;
    u64 first_data_sec = fat_tab_sec
                       + record->fat_num * fat_siz
                       + (record->roots * 32 + (record->sec_siz - 1)) / record->sec_siz; // root_dir_sectors
    u64 root_sec = (record->root_clus - 2) * record->sec_perclus + first_data_sec;
    DEBUGK(K_INIT,
           "fat32 -> tab : %#x (%u,%u) , first_data_sec : %#x , root_sec : %#x "
           "{%d}\n",
           fat_tab_sec, record->fat_num, fat_siz, first_data_sec, root_sec,
           record->sec_perclus);

    /* 记录文件系统信息 */
    sysinfo_t *sys = malloc(sizeof(sysinfo_t));
    sys->dev = hd;
    sys->mbr = mbr;
    sys->record = record;
    sys->pentry = pentry;
    sys->fat_sec = fat_tab_sec;
    sys->first_data_sec = first_data_sec;
    sys->free_cnt = free_cnt;
    sys->free_next = free_next;
    
    /* 初始化此文件系统根节点 */
    node_t *ori = malloc(sizeof(node_t));
    ori->attr = NA_DIR;
    ori->name = "/";
    ori->root = ori;
    ori->child = NULL;
    ori->parent = ori;
    ori->sys = sys;
    ori->systype = FS_FAT32;
    ori->idx = root_sec;
    ori->siz = 0;

    /* FAT32 文件系统接口 */
    ori->opts = &__fat32_opts;

    return ori;
fail:
    if (record)
        free(record);
    return NULL;
}

//
// 好了, 下面是 entry 操作
//

#define ALIGN_UP(target, base) ((base) * ((target + base - 1) / base))
#define ALIGN_DOWN(target, base) ((base) * (target / base))

#define DUMP_IC(sys, sec)    (((sec) - (sys)->first_data_sec) / (sys)->record->sec_perclus + 2)
#define DUMP_IS(sys, clus)   (((clus) - 2) * (sys)->record->sec_perclus + (sys)->first_data_sec)

/* 获取此 clus 处于 FAT1 中的地址 */
#define TAB_IS(sys, clus) (sys->fat_sec + (clus * 4) / SECT_SIZ)

#define FAT_ALOC_NR  (SECT_SIZ / sizeof(u32))      // 一扇区 FAT 表的索引容量
#define FAT_ENTRY_NR (SECT_SIZ / sizeof(sentry_t)) // 一扇区 Entry 容量

typedef struct
{
    u32 clus;
    u64 idx;
    list_t list;
} locate_t;

/* ls : a ptr to a list head */
#define LOCATOR_ADD(ls, c, i)                         \
        do {                                          \
            locate_t *l = malloc(sizeof(locate_t));   \
            l->clus = c;                              \
            l->idx = i;                               \
            list_insert_after((ls), &l->list);        \
        } while (false);

#define LOCATOR_CLR(ls)                               \
        do {                                          \
            locate_t *l;                              \
            while (!list_empty (ls)) {                \
                l = CR((ls)->next, locate_t, list);   \
                free (l);                             \
                list_remove((ls)->next);              \
            }                                         \
        } while (false);                              \

/* 这里使用一种抽象是为了缩小代码体积, 以及简化一些重复操作 */
typedef struct
{
    sysinfo_t *sys;   // 文件系统信息
    struct {          // 存储着一些位置信息
        u32 clus;     // 本结构所代表的 entry 的簇号
                      // 如果是 dir_entry , 是索引此目录下文件的那几个簇的起始簇号
                      // 如果是 archive_entry , 是此文件内容的起始簇簇号
        list_t list;  // 描述各个 entry 位于哪个簇, 是第几个 entry
    } locator;
    stack_t ents;     // entry 栈
    node_t *node;     // entry 被解析后生成的 vfs node
} lookup_t;

static inline lookup_t *_lkp_origin (node_t *n, lookup_t *lkp)
{
    if (!lkp)
        lkp = malloc(sizeof(lookup_t));

    lkp->locator.clus = DUMP_IC(((sysinfo_t *)n->sys), n->idx);
    stack_init (&lkp->ents);
    list_init  (&lkp->locator.list);
    lkp->node = n;
    lkp->sys = n->sys;

    return lkp;
}

#define LKP_ORI(n) \
        (_lkp_origin (n, NULL))

static size_t _alloc_part (sysinfo_t *sys);
static size_t _expand (node_t *this, size_t siz);
static size_t _expand_part (node_t *this, size_t cnt, bool append);

/*
    在一个父 entry 下查找子项, 并生成 Lkp, 包含信息 (行为) 如下:
      - 一个栈
        - 保存着 entry 信息, 包含长短目录项
      - 一个 Locator 定位子
        - 一个 描述当前 entry 所指向的 文件/目录 的簇号 (Cluster)
        - 一个 描述 entry 在文件系统中的具体位置的链表  (List)
      - vfs 抽象的子节点
          - 只是在物理文件系统层面的初始化 -> 不会参与 节点 有关虚拟文件系统部分的初始化
      - vfs 环境 (所处的文件系统信息) -> sys
*/
static lookup_t *lookup_entry (lookup_t *parent, char *target, size_t idx);

enum {
    LKPS_NONE   = 0x00,
    LKPS_UPDATE = 0x01,
    LKPS_CREATE = 0x02,
    LKPS_CHILD  = 0x04,
    LKPS_ERASE  = 0x08
};

static void      lookup_save (lookup_t *lkp, stack_t *update, int opt);

//

static sentry_t *_entry_dup (sentry_t *ori)
{
    sentry_t *buf = malloc(sizeof(sentry_t));
    return memcpy (buf, ori, sizeof(sentry_t));
}

static u64 _addr_get (sysinfo_t *sys, sentry_t *sent)
{
    u64 addr = sys->first_data_sec
             + sys->record->sec_perclus * ((sent->clus_low | (u32)sent->clus_high << 16) - 2);
    return addr;
}

#define _(m, ptr)                                     \
    for (int i = 0; i < sizeof(m) / sizeof(*m); i++)  \
        if (m[i] == 0xFFFF)                           \
            break;                                    \
        else                                          \
            *ptr++ = m[i];                            \

static inline char *_parse_namel (lentry_t *lent, char **buf)
{
    char *ptr = *buf;

    _(lent->name1, ptr);
    _(lent->name2, ptr);
    _(lent->name3, ptr);

    return *buf = ptr;
}

#undef _

static inline char *_parse_name (sentry_t *sent, char *buf)
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

static inline char *_make_namel (lentry_t *lent, char **buf)
{
    char *ptr = *buf;

    u16 tmp[_LEN_LONGW];
    for (int i = 0 ; i < _LEN_LONGW ; i++) {
        if (*ptr == 0) {
            tmp[i] = 0x0000;
            for (i = i + 1 ; i < _LEN_LONGW ; i++ )
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

#define ENTRY_VALID(entry) ((entry).name[0] != 0 && (u8)(entry).name[0] != 0xe5)
#define ENTRY_ERASE(entry) ((entry).name[0] =  0)

static void _destory_handler (void *payload)
{
    free (payload);
}

static inline u8 _cksum (char *sent) 
{ 
    u8 sum = 0;
    u8 *ptr = (u8 *)sent;
    
    for (short len = 11 ; len != 0 ; len--) {
        // NOTE: The operation is an unsigned char rotate right
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *ptr++;
    }
    return sum; 
} 

#define ORDER_LAST 0x40

/* 调用函数来完成也许是无必要的, 即使编译器会优化 */
#define UPPER_CASE(c) ('a' <= c && c <= 'z' ? c - 32 : c)
#define LOWER_CASE(c) ('A' <= c && c <= 'Z' ? c + 32 : c)

static stack_t *_make_entry (node_t *target)
{
    stack_t *stk = stack_init (NULL);
    ASSERTK (stk != NULL);
    
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
    if (len > LEN_EXT + LEN_NAME)
    {
        longer = true;
        goto make;
    }

    /*
       NOTE: All the characters which are not supported by short entry will be replaced by '_'
    */
    size_t NrDots = 0;
    for (int i = 0 ; i < len ; i++) {
        switch (name[i]) {
            /* Unsupported chars */
            case '+': case ',': case ';':
            case '=': case '[': case ']':
                name[i] = '_';
            
            case ' ':
            case 'a'...'z':
                longer = true;
                goto make;
            
            case '.':
                if (++NrDots > 1)
                    longer = true;
            break;

            default: break;
        }
    }
    
    //

    int ext_start, name_start;
    char _name[LEN_NAME],
         _ext [LEN_EXT ];
    int  _namei, _exti;
make:
    memset (_name, ' ', sizeof(_name));
    memset (_ext , ' ', sizeof(_ext ));
    /* 对于 `.XXXX` 的 情况, `.XXXX` 不算做拓展名 */
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

    /* 开始填充 */
    _exti = 0;
    _namei = 0;
    if (ext_start != 0) {
        for (int i = ext_start ; i < len ; i++) {
            char append = 0;
            switch (name[i]) {
                case ' ':
                    continue;

                default:
                    append = UPPER_CASE(name[i]);
                break;
            }

            if (_exti < LEN_NAME)
                _ext[_exti++] = append;
        }
    } else {
        /* 不阻挡 name 的初始化 */
        ext_start = 0xff;
    }

    for (int i = name_start ; i < len && i < ext_start ; i++) {
        char append = 0;
        switch (name[i]) {
            case '.':
            case ' ':
                continue;

            default:
                append = UPPER_CASE(name[i]);
            break;
        }

        if (_namei < LEN_NAME)
            _name[_namei++] = append;
    }
    
    free (name);

    sysinfo_t *sys = target->sys;

    sentry_t *sent = malloc (sizeof(sentry_t));
    memset (sent, 0, sizeof(sentry_t));
    memcpy (sent->name, _name, LEN_NAME);
    memcpy (sent->name + LEN_NAME, _ext, LEN_EXT);
    sent->filesz = target->siz;
    if (target->idx) {
        sent->clus_high = DUMP_IC(sys, target->idx) >> 16;
        sent->clus_low  = DUMP_IC(sys, target->idx) & 0xFFFF;
    }
    sent->attr = (target->attr & NA_REG ? FA_ARCHIVE : FA_DIR);
    stack_push (stk, sent);

    /* 接下来是长目录的主场 (有的话...) */

    u8 cksum = _cksum (sent->name);
    if (longer) {
        int cnt = DIV_ROUND_UP(len, _LEN_LONGW);

        char *ptr = target->name;
        lentry_t *lent;
        for (int i = 0 ; i < cnt ; i++)
        {
            lent = malloc (sizeof(lentry_t));
            memset (lent, 0, sizeof(lentry_t));
            memset (lent->name1, 0xFF, sizeof(lent->name1));
            memset (lent->name2, 0xFF, sizeof(lent->name2));
            memset (lent->name3, 0xFF, sizeof(lent->name3));

            _make_namel (lent, &ptr);

            // TODO : Check sum
            lent->attr = FA_LONG;
            lent->nr = i + 1;
            lent->cksum_short = cksum;
            
            stack_push (stk, lent);
        }

        /* Set mask for the last entry */
        lent->nr |= ORDER_LAST;
    }

    return stk;
}

static node_t *_analysis_entry (sysinfo_t *sys, stack_t *stk)
{
    ASSERTK (!stack_empty (stk));
    stacki_t *iter = stacki(stk, iter);

    node_t *n = malloc (sizeof(node_t));
    memset (n, 0, sizeof(node_t));

    sentry_t *main = stacki_data(iter);
                     stacki_next(iter);

    n->siz = main->filesz;
    n->attr = 0;
    if (main->attr & FA_DIR)
        n->attr |= NA_DIR;
    else if (main->attr & FA_ARCHIVE)
        n->attr |= NA_REG;

    if (main->clus_low || main->clus_high)
        n->idx = _addr_get (sys, main);

    size_t cnt = stack_siz(stk) - 1;
    /* Only has a main entry (short entry) */
    if (cnt == 0)
    {
        /* Allocate memory for it in stack in order to improve the speed */
        char buf[LEN_NAME + LEN_EXT];
        memset (buf, 0, sizeof(buf));
        /* Do parsing & copying... */
        n->name = strdup(_parse_name (main, buf));
    }
    else
    {
        n->name = malloc(_LEN_LONGC * cnt);
        
        do {
            char *ptr = n->name;
            while (cnt--)
            {
                _parse_namel(stacki_data(iter), &ptr);
                             stacki_next(iter);
            }
        } while (false);
    }

    return n;
}

/* 根据 stk 来申请, 结果保存到 lkp 的 list 中 */
static void lookup_alloc (lookup_t *parent, lookup_t *lkp, stack_t *stk)
{
    sysinfo_t *sys = parent->sys;

    u32 curr = parent->locator.clus;
    u32 prev = parent->locator.clus;
    size_t start_idx = 0, curr_idx  = 0;

    size_t cnt = stack_siz (stk);
    stack_init (&lkp->ents);
    list_init  (&lkp->locator.list);
    
    bool hit = false;
    buffer_t *entsblk, *idxblk;
    while (true)
    {
        entsblk = bread(sys->dev, DUMP_IS(sys, curr));
        idxblk = bread(sys->dev, TAB_IS(sys, curr));
        sentry_t *ents =  entsblk->blk;
        u32 *idxes = idxblk->blk;

        for (int i = 0 ; i < FAT_ENTRY_NR ; i++, curr_idx++)
        {
            /* 希望它 free */
            if (ENTRY_VALID(ents[i])) {
                prev = curr;
                start_idx = curr_idx + 1;
                LOCATOR_CLR(&lkp->locator.list);
                continue;
            }

            LOCATOR_ADD(&lkp->locator.list, curr, i);
            if (curr_idx - start_idx + 1 == cnt) {
                hit = true;
                break;
            }
        }

        prev = curr;
        curr = idxes[curr % FAT_ALOC_NR];
        brelse(idxblk);
        brelse(entsblk);

        // reaches the end
        if (IS_EOC(curr))
            break;
    }
    
    if (!hit) {
        _expand_part (parent->node, 1, true);
        lookup_alloc (parent, lkp, stk);
    }
}

static void _lookup_release (lookup_t *lkp)
{
    if (!stack_empty (&lkp->ents))
        stack_clear (&lkp->ents);

    /* TODO: 可行性检查 -> 是否需要释放 */
    if (lkp->node)
        vfs_release (lkp->node);
}

_UTIL_CMP();

static lookup_t *lookup_entry (lookup_t *parent, char *target, size_t idx)
{
    lookup_t *lkp = malloc (sizeof(lookup_t));

    stack_init (&lkp->ents);
    stack_set  (&lkp->ents, _destory_handler, NULL);
    list_init  (&lkp->locator.list);
    
    sysinfo_t *sys = parent->sys;
    u32 curr = parent->locator.clus;

    bool find = false;

    buffer_t *entsblk, *idxblk;
    while (true)
    {
        entsblk = bread(sys->dev, DUMP_IS(sys, curr));
        idxblk = bread(sys->dev, TAB_IS(sys, curr));
        sentry_t *ents = entsblk->blk;
        u32 *idxes = idxblk->blk;

        for (int i = 0 ; i < SECT_SIZ / sizeof(sentry_t) ; i++)
        {
            /* 这个文件已经被删除了 (不存在) */
            if (!ENTRY_VALID(ents[i]))
                goto entry_next;

            LOCATOR_ADD (&lkp->locator.list, curr, i);
            if (ents[i].attr & FA_LONG)
            {
                lentry_t *lent, *prev;

                lent = (void *)&ents[i];
                if (~lent->nr & ORDER_LAST)
                    goto entry_next;

                prev = stack_top(&lkp->ents);
                ASSERTK (prev ? lent->cksum_short == prev->cksum_short : true);
                stack_push (&lkp->ents, _entry_dup (&ents[i]));

                goto entry_next;
            }

            if (!stack_empty (&lkp->ents)) {
                lentry_t *prev = stack_top (&lkp->ents);

                u8 cksum = _cksum (ents[i].name);
                if (prev->cksum_short != cksum) {
                    /* Starts from the origin */
                    stack_clear (&lkp->ents);
                    goto entry_next;
                }
            }

            /* store the short entry so that we can handle
               it and its attached ents at the same time */
            stack_push (&lkp->ents, &ents[i]);

            lkp->node = _analysis_entry (sys, &lkp->ents);
            if (target == NULL) {
                if (idx-- == 0) {
                    find = true;
                    break;
                }
                goto lkp_rst;
            }

            size_t cmpsiz = (size_t)(strchrnul (target, '/') - target);
            if (_cmp (target, lkp->node->name)) {
                find = true;
                break;
            }

        lkp_rst:
            vfs_release (lkp->node);
            lkp->node = NULL;
            stack_clear (&lkp->ents);
            LOCATOR_CLR(&lkp->locator.list);

        /* Handle the next entry we have read before */
        entry_next:
            continue;
        }

        curr = idxes[curr % FAT_ALOC_NR];
        brelse(idxblk);
        brelse(entsblk);

        // reaches the end
        if (IS_EOC(curr) || find)
            break;
    }

    if (!lkp->node) {
        free (lkp);
        lkp = NULL;
    } else {
        lkp->sys = parent->sys;
        lkp->locator.clus = DUMP_IC(sys, lkp->node->idx);
    }
    return lkp;
}

static void _lookup_all (lookup_t *parent, stack_t *child)
{
    sysinfo_t *sys = parent->sys;
    u32 curr = parent->locator.clus;

    buffer_t *entsblk, *idxblk;
    while (true)
    {
        entsblk = bread(sys->dev, DUMP_IS(sys, curr));
        idxblk = bread(sys->dev, TAB_IS(sys, curr));
        sentry_t *ents = entsblk->blk;
        u32 *idxes = idxblk->blk;

        for (int i = 0 ; i < SECT_SIZ / sizeof(sentry_t) ; i++)
        {
            /* 这个文件已经被删除了 (不存在) */
            if (!ENTRY_VALID(ents[i]))
                goto entry_next;
            stack_push (child, _entry_dup(&ents[i]));

        entry_next:
            continue;
        }

        curr = idxes[curr % FAT_ALOC_NR];
        brelse(idxblk);
        brelse(entsblk);
        if (IS_EOC(curr))
            break;
    }
}

/* Write a stack `Update` according to the list in `Origin` */
static void lookup_save_common (lookup_t *ori, stack_t *update)
{
    sysinfo_t *sys = ori->sys;

    list_t *head = &ori->locator.list;

    /* 逆向遍历, 顺应 stack */
    sentry_t *ents = malloc(SECT_SIZ);
    buffer_t *entsblk;
    for (list_t *ptr = head->prev ; ptr != head ; ptr = ptr->prev) {
        locate_t *lct = CR (ptr, locate_t, list);
        entsblk = bread(sys->dev, DUMP_IS(sys, lct->clus));

        ASSERTK (lct->idx < 16);
        ASSERTK (stack_siz(update) != 0);
        sentry_t *member = stack_top(update);
        sentry_t *ents = entsblk->blk;
        memcpy (&ents[lct->idx], member, sizeof(sentry_t));
        stack_pop(update);

        bdirty(entsblk, true);
        brelse(entsblk);
    }
}

static void lookup_save_child (lookup_t *ori, stack_t *child)
{
    sysinfo_t *sys = ori->sys;
    u32 curr = ori->locator.clus;

    buffer_t *entsblk, *idxblk;
    while (true)
    {
        idxblk = bread(sys->dev, TAB_IS(sys, curr));
        entsblk = bread(sys->dev, DUMP_IS(sys, curr));
        sentry_t *ents = entsblk->blk;
        u32 *idxes = idxblk->blk;

        for (int i = 0 ; i < SECT_SIZ / sizeof(sentry_t) ; i++)
        {
            if (!stack_empty (child)) {
                void *member = stack_top (child);
                memcpy (&ents[i], member, sizeof(sentry_t));
                stack_pop (child);
            } else {
                memset (&ents[i], 0, sizeof(sentry_t));
            }
        }

        curr = idxes[curr % FAT_ALOC_NR];
        brelse(idxblk);
        bdirty(entsblk, true);
        brelse(entsblk);
        if (IS_EOC(curr))
            break;
    }
}

/* Erase ents which `Origin`'s list describes */
static void lookup_save_erase (lookup_t *ori)
{
    sysinfo_t *sys = ori->sys;

    u32 prev = 0;
    list_t *head = &ori->locator.list;

    /* 逆向遍历, 顺应 stack */
    sentry_t *ents = malloc (SECT_SIZ);
    for (list_t *ptr = head->prev ; ptr != head ; ptr = ptr->prev) {
        locate_t *lct = CR (ptr, locate_t, list);
        buffer_t *entsblk = bread(sys->dev, DUMP_IS(sys, lct->clus));
        sentry_t *ents = entsblk->blk;
        ASSERTK (lct->idx < 16);
        ENTRY_ERASE(ents[lct->idx]);
        bdirty(entsblk, true);
        brelse(entsblk);
    }
}

/*
    当 Opt == LKPS_CHILD, stack 代表 Lkp目录 的子项组成的栈
*/
static void lookup_save (lookup_t *lkp, stack_t *stk, int opt)
{
    ASSERTK (opt != LKPS_NONE);

    if (opt == LKPS_ERASE)
    {
        ASSERTK (stk == NULL);
        lookup_save_erase (lkp);
    }
    /* Write an entry created */
    else if (opt == LKPS_CREATE)
    {
        ASSERTK (stack_siz (&lkp->ents) == 0);
        lookup_save_common (lkp, stk);
    }
    else if (opt == LKPS_CHILD)
    {
        lookup_save_child (lkp, stk);
    }
    /* Overwrite the original */
    else if (opt == LKPS_UPDATE)
    {
        if (stack_siz(&lkp->ents) == stack_siz(stk)) {
            lookup_save_common (lkp, stk);
        }
        else { /* space will be taken up may be too small, in spite of the original */
            lookup_save_erase (lkp);
            ASSERTK (lkp->node->parent != NULL);
            lookup_t *Prt = LKP_ORI(lkp->node->parent);
            lookup_alloc (Prt, lkp, stk);
            lookup_save  (lkp, stk, opt);
        }
    }
}

/* Common sync */
static void _com_sync (node_t *before, node_t *after)
{
    lookup_t *parent = LKP_ORI(before->parent);
    lookup_t *origin = lookup_entry (parent, before->name, 0);
    origin->sys = parent->sys;

    stack_t *stk = _make_entry (after);
    lookup_save (origin, stk, LKPS_UPDATE);
}

static sentry_t dirhead_1 = { ".          ", FA_DIR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static sentry_t dirhead_2 = { "..         ", FA_DIR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static node_t *_create (node_t *parent, char *name, int attr)
{
    node_t *child = malloc (sizeof(node_t));
    memset (child, 0, sizeof(node_t));

    child->sys     = parent->sys;
    child->systype = parent->systype;
    child->opts   = parent->opts;

    child->name = strdup(name);
    child->attr = attr;
    
    /* update parent's & child's nodes */
    child->next = parent->child;
    parent->child = child;
    child->parent = parent;

    child->siz = 0;
    /* allocate an address when it's about to be written */
    child->idx = 0;
    if (child->attr & NA_DIR)
        child->idx = _alloc_part (child->sys);

    /* Write our child node into disk (media) */
    lookup_t *par = LKP_ORI(parent);
    lookup_t *lkp = LKP_ORI(child);

    stack_t *stk = _make_entry(child);
    lookup_alloc (par, lkp, stk);
    lookup_save  (lkp, stk, LKPS_CREATE);
    
    /* 目录开头是需要一些预设的 entry 的, 比如 '.' '..' */
    if (child->attr & NA_DIR) {
        lookup_t *lkpchd = LKP_ORI(child);
        stack_t *init = stack_init (NULL);

        stack_push(init, &dirhead_2);
        stack_push(init, &dirhead_1);
        lookup_save (lkpchd, init, LKPS_CHILD);
            
        stack_fini (init);
    }

    stack_fini (stk);
    return child;
}

//
// 文件系统操作
//

_UTIL_NEXT();
_UTIL_PATH_DIR();

/*
    路径游走 , 我第一次听到这个好听的名字是在 lunaixsky 的视频里面, 这里直接沿用了!

    @param Start  父目录
    @param Path  搜索对象文件名指针
    @param Last  最后一个可以被索引的已加载的文件节点
*/
static node_t *_fat32_open (node_t *start, char *path)
{
    sysinfo_t *sys = start->sys;
    lookup_t *lkp = LKP_ORI(start);
    node_t *res = NULL;

    lkp = lookup_entry(lkp, path, 0);
    if (!lkp)
        goto end;
    res = lkp->node;

    stack_fini(&lkp->ents);
    free(lkp);

    res->parent = start;
    res->opts = start->opts;
    res->sys = start->sys;
    res->systype = start->systype;

    res->next = start->child;
    start->child = res;

end:
    return res;
}

static int fat32_open (node_t *parent, char *path, u64 args, node_t **result)
{
    int ret = 0;
    node_t *opened;

    opened = _fat32_open(parent, path);
    if (opened)
        goto end;

    if (args & VFS_CREATE)
    {
        /*
           按照习惯, 默认只能在一个已经存在的目录下创建新的节点,
           但是如果 Path 不到头, 就说明此文件的父目录也不存在, 所以直接返回 NULL
        */
        if (*_next(path) != 0) {
            ret = -ENOENT;
            goto end;
        }

        int attr = 0;
        if (args & VFS_DIR)
            attr |= NA_DIR;
        else
            attr |= NA_REG;
        opened = _create (parent, path, attr);
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

/* Close a node , and delete its sub-dir -> call the public handler */
static int fat32_close (node_t *this)
{
    return vfs_release (this);
}

static int _read_content (node_t *n, void *buf, size_t readsiz, size_t offset)
{
    sysinfo_t *sys = n->sys;

    if (readsiz == 0)
        return 0;
    if (offset >= n->siz)
        return EOF;
    
    u32 curr = DUMP_IC(sys, n->idx);
    size_t offset_sect = offset / SECT_SIZ,
           offset_byte = offset % SECT_SIZ;
    
    size_t real = MIN(n->siz, offset + readsiz) - offset;
    size_t cnt  = DIV_ROUND_UP(real, sys->record->sec_siz);
    size_t siz  = real;

    for (int i = 0 ;  ; i++) {
        if (i >= offset_sect) {
            buffer_t *tmp = bread(sys->dev, DUMP_IS(sys, curr));
            size_t cpysiz = MIN(siz, offset_byte != 0 ? SECT_SIZ - offset_byte : MIN(siz, SECT_SIZ));
            memcpy (buf, tmp->blk + offset_byte, cpysiz);
            brelse(tmp);

            offset_byte = 0;

            buf += cpysiz;
            cnt--;
            if (cnt == 0)
                goto end;
            siz -= cpysiz;
        }
        
        buffer_t *idxblk = bread(sys->dev, TAB_IS(sys, curr));
        u32 *idxes = idxblk->blk;
        curr = idxes[curr % FAT_ALOC_NR];
        brelse(idxblk);

        // reaches the end
        if (IS_EOC(curr))
            break;
    }

end:
    return real;
}

static int fat32_read (node_t *this, void *buf, size_t siz, size_t offset)
{
    return _read_content (this, buf, siz, offset);
}

static size_t _alloc_part (sysinfo_t *sys)
{
    buffer_t *idxblk;
    size_t res = 0, curr = DUMP_IC(sys, sys->first_data_sec) + sys->free_next - 1;
    for ( ; ; )
    {
        idxblk = bread(sys->dev, TAB_IS(sys, curr));
        u32 *idxes = idxblk->blk;

        for (int i = curr % FAT_ALOC_NR ; i < FAT_ALOC_NR ; i++, curr++) {
            if (idxes[i] == 0) {
                idxes[i] = EOC;
                sys->free_next = curr + 1;
                res = curr;
                bdirty(idxblk, true);
                brelse(idxblk);
                goto fini;
            }
        }
        brelse(idxblk);
    }

fini:
    return res == 0 ? 0 : DUMP_IS(sys, res);
}

/* 拓展 This 所指向的实体, 在原有的基础上拓宽/拓宽到 Cnt 个扇区 */
static size_t _expand_part (node_t *n, size_t cnt, bool append)
{
    sysinfo_t *sys = n->sys;
    u32 curr, end;
    
    buffer_t *curblk;
    u32 *idxes_cur, *idxes_end;

    if (n->idx == 0) {
        n->idx = _alloc_part (n->sys);
        if (--cnt == 0)
            goto exit;
    }

    ASSERTK (n->idx >= sys->first_data_sec);

    // TODO: starts from head
    curr = DUMP_IC(sys, n->idx);

    while (true)
    {
        curblk = bread(sys->dev, TAB_IS(sys, curr));
        idxes_cur = curblk->blk;

        u32 next = idxes_cur[curr % FAT_ALOC_NR];
        if (IS_EOC(next))
            break;
        else
            curr = next;
        
        brelse(curblk);
        if (!append)
            if (--cnt == 0)
                goto exit;
    }
    end = curr;
    idxes_end = idxes_cur;

    for ( ; ; curr++)
    {
        // next sector of FAT1
        if (curr % FAT_ALOC_NR == 0) {
            bdirty(curblk, true);
            brelse(curblk);
            curblk = bread(sys->dev, TAB_IS(sys, curr));
            idxes_cur = curblk->blk;
        }

        // free data sector to use
        if (!idxes_cur[curr % FAT_ALOC_NR]) {
            // zero the free sector
            buffer_t *zero = bread(sys->dev, DUMP_IS(sys, curr));
            memset(zero->blk, 0, SECT_SIZ);
            bdirty(zero, true);
            brelse(zero);

            idxes_end[end % FAT_ALOC_NR] = curr;
            if (idxes_end != idxes_cur) {
                // 这个空闲项已经作为一个节点链接到上一个索引,
                // 也就是上一个结尾, 此时的结尾应该被更新
                idxes_end = idxes_cur;
            }
            end = curr;
            
            cnt -= 1;
        }

        if (cnt == 0) {
            idxes_end[end % FAT_ALOC_NR] = EOC;
            /* Save changes to disk */
            brelse(curblk);
            break;
        }
    }

exit:
    return cnt;
}

/* NOTE : Do not opearte `This` in this function! */
static size_t _expand (node_t *this, size_t siz)
{
    sysinfo_t *sys = this->sys;

    if (ALIGN_UP(this->siz, SECT_SIZ) >= siz)
        return siz;

    size_t cnt = DIV_ROUND_UP(siz - ALIGN_UP(this->siz, SECT_SIZ), SECT_SIZ);
    size_t res = _expand_part (this, cnt, false) * SECT_SIZ;
    ASSERTK (res == 0);

    return siz;
}

/*
   战术: 扩容为先
*/
static int _write_content (node_t *n, void *buf, size_t writesiz, size_t offset)
{
    if (writesiz + offset >= n->siz)
        n->siz = _expand (n, writesiz + offset);

    sysinfo_t *sys = n->sys;

    u32 curr = DUMP_IC(sys, n->idx);

    size_t offset_sect = offset / SECT_SIZ,
           offset_byte = offset % SECT_SIZ;

    size_t cnt = DIV_ROUND_UP(writesiz, SECT_SIZ);
    size_t siz = writesiz;
    for (size_t i = 0 ;  ; i++)
    {
        if (i >= offset_sect) {
            buffer_t *ori = bread(sys->dev, DUMP_IS(sys, curr));
            /* MIN 的意义在于 : Writesiz 可能比一个扇区的大小要小 */
            size_t cpysiz = MIN(siz, offset_byte != 0 ? SECT_SIZ - offset_byte : MIN(siz, SECT_SIZ));
            memcpy (ori->blk + offset_byte, buf, cpysiz);
            bdirty(ori, true);
            brelse(ori);

            /* After using ByteOffset the first time, set it to zero */
            offset_byte = 0;

            buf += cpysiz;
            cnt--;
            if (cnt == 0)
                goto end;
            siz -= cpysiz;
        }

        buffer_t *idxblk = bread(sys->dev, TAB_IS(sys, curr));
        u32 *idxes = idxblk->blk;
        curr = idxes[curr % FAT_ALOC_NR];
        bdirty(idxblk, true);
        brelse(idxblk);

        // reaches the end
        if (IS_EOC(curr))
            break;
    }

end:
    _com_sync (n, n);
    return writesiz;
}

static int fat32_write (node_t *this, void *buf, size_t siz, size_t offset)
{
    return _write_content (this, buf, siz, offset);
}

#define IDX_ERASE(idx, stat) ({ u32 old = (idx) ; (idx) = (stat) ; old; })

/*
    TODO : Test it

    cut 指定是否是截断操作, cut == 0 仅当 start 为文件开头, 且释放所有数据块时
*/
static void _release_data (sysinfo_t *sys, u32 start, bool cut)
{
    u32 curr = start;
    u32 stat = cut ? EOC : 0;
    
    while (true)
    {
        buffer_t *idxblk = bread(sys->dev, TAB_IS(sys, curr));
        u32 *idxes = idxblk->blk;
        if (IS_EOC(idxes[curr % FAT_ALOC_NR]))
        {
            bdirty(idxblk, true);
            brelse(idxblk);
            break;
        }

        curr = IDX_ERASE(idxes[curr % FAT_ALOC_NR], stat);
        bdirty(idxblk, true);
        brelse(idxblk);
        stat = 0;
    }
}

static int fat32_remove (node_t *this)
{
    sysinfo_t *sys = this->sys;

    lookup_t *par = LKP_ORI (this->parent);
    lookup_t *lkp = lookup_entry (par, this->name, 0);
    lookup_save (lkp, NULL, LKPS_ERASE);

    /* 释放数据区 */
    _release_data (lkp->sys, lkp->locator.clus, false);
    return fat32_close (this);
}

static int fat32_truncate (node_t *this, size_t offset)
{
    if (offset > this->siz)
        this->siz = _expand (this, offset);
    else {
        sysinfo_t *sys = this->sys;
        /* 当 Offset == 0 时, 释放掉所有簇 */
        bool all = false;
        if (offset == 0) {
            all = true;
            this->idx = 0;
        }
        _release_data (
            sys, DUMP_IC(sys, this->idx),
            all
        );
        this->siz = offset;
    }
    _com_sync (this, this);

    return 0;
}

/* TODO : ret-val */
static int fat32_mkdir (node_t *parent, char *name)
{
   node_t *dir = _create (parent, name, NA_DIR);
   if (!dir)
       return -1;

   return 0;
}

static int fat32_readdir (node_t *this, node_t **res, size_t idx)
{
    lookup_t *lkp, *par = LKP_ORI(this);
    /* 根据索引查找之后对其进行 vfs 层面的链接 */

    *res = NULL;

    lkp = lookup_entry(par, NULL, idx);
    if (lkp == NULL)
        return -1;

    node_t *chd = lkp->node;
    if (vfs_test(this, chd->name, NULL, NULL)) {
        vfs_release(chd);
    } else {
        /* 物理文件系统 "无关" 的基本信息 */
        chd->parent = this;
        chd->opts = this->opts;
        chd->sys = this->sys;
        chd->systype = this->systype;
        /* 挂载到父节点 */
        chd->next = this->child;
        this->child = chd;
    }

    stack_fini(&lkp->ents);
    free(lkp);

    *res = chd;
    return 0;
}


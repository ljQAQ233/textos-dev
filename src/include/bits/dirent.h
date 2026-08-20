#ifdef __NEED_dir_t
#define __DEFINED_dir_t
typedef struct
{
    int idx;
    int type;
    ino_t ino;
    size_t siz;
    char name[];
} dir_t;
#endif

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK     10
#define DT_SOCK    12
#define DT_WHT     14

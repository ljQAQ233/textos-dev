#include <textos/boot.h>
#include <textos/boot/mb1.h>
#include <textos/cmdline.h>
#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/mm/heap.h>
#include <textos/mm/map.h>
#include <textos/mm/pmm.h>

static char *_cmdline;
static size_t _cmdline_size;
static size_t _cmdline_pages;

#define PREFACE_STR SIGN_64(033, 'K', 'C', 'M', 'D', 'S', 'T', 'R')

static bool prefix_cmp(const char *str, const char *pat, size_t n)
{
    while (n--)
        if ((*str | 32) != (*pat | 32)) return false;
    return true;
}

static bool key_cmp(const char *str, const char *key)
{
    while (*str != '=' && *str) {
        if (*str != *key) return false;
        str++, key++;
    }
    return true;
}

void __cmdline_pre()
{
    struct multiboot_info *b = binfo_get();
    if (~b->flags & MULTIBOOT_INFO_CMDLINE) {
        DEBUGK(K_INFO, "cmdline not provided by multiboot\n");
        return;
    }
    char *cmdline = (char *)(addr_t)b->cmdline;

    _cmdline_size = strlen(cmdline) + 1;
    if (_cmdline_size >= __kern_cmdline_max - __kern_cmdline_base)
        PANIC("cmdline too long\n");

    _cmdline_pages = DIV_ROUND_UP(_cmdline_size, PAGE_SIZE);
    _cmdline = (char *)pmm_allocpages(_cmdline_pages);
    if (_cmdline == NULL)
        PANIC("cannot backup cmdline due to a lack of pages\n");
    memcpy(_cmdline, cmdline, _cmdline_size);

    {
        char snapshot[32];
        strncpy(snapshot, _cmdline, sizeof(snapshot) - 1);
        snapshot[sizeof(snapshot) - 1] = '\0';
        DEBUGK(K_INFO, "cmdline accepts %lu bytes totally\n", _cmdline_size);
        DEBUGK(K_INFO, "cmdline snapshot = '%s'\n", b->cmdline);
    }
}

void __cmdline_tovmm()
{
    addr_t pa = (addr_t)_cmdline;
    addr_t va = __kern_cmdline_base;
    vmap_map(pa, va, _cmdline_pages, PE_P | PE_RW);
    _cmdline = (char *)va;
}

//

void cmdline_get_raw(const char **raw, size_t *size)
{
    *raw = _cmdline;
    *size = _cmdline_size;
}

bool cmdline_search(const char *key, const char **v, const char **vend)
{
    char *s = _cmdline;
    for (;;) {
        if (!*s) break;
        s = strchrnul(s, ' ') + 1;
        if (!*s) break;
        if (key_cmp(s, key)) {
            if (!v || !vend) return true;
            const char *space = strchrnul(s, ' ');
            const char *equal = strchrnul(s, '=');
            if (space > equal) {
                *v = equal + 1;
                *vend = space;
            } else {
                *v = *vend = NULL;
            }
            return true;
        }
    }
    return false;
}

//

void cmdline_free_str(char *str)
{
    char *mem = str - 8;
    if (*(uint64_t *)mem == PREFACE_STR) free(mem);
}

int cmdline_get_buf(const char *key, char *buf, size_t *size)
{
    const char *v, *vend;
    if (!cmdline_search(key, &v, &vend)) return -ENOENT;
    if (!v) v = "";
    const size_t len = vend - v;
    if (*size < len + 1) {
        *size = len + 1;
        return -ERANGE;
    }
    memcpy(buf, v, len);
    buf[len] = '\0';
    return 0;
}

int cmdline_get_str(const char *key, char **str)
{
    const char *v, *vend;
    if (!cmdline_search(key, &v, &vend)) return -ENOENT;
    if (!v) v = "";
    const size_t len = vend - v;
    char *mem = malloc(8 + len + 1);
    if (!mem) return -ENOMEM;
    *(uint64_t *)mem = PREFACE_STR;
    *str = mem + 8;
    memcpy(*str, v, len);
    (*str)[len] = '\0';
    return 0;
}

int cmdline_get_u64(const char *key, uint64_t *val)
{
    const char *v, *vend;
    if (!cmdline_search(key, &v, &vend)) return -ENOENT;
    const char *p = v;
    uint64_t x = 0;
    uint64_t sign = 1;
    if (*p == '-')
        sign = -1, p++;
    else if (*p == '+')
        sign = 1, p++;
    for (; '0' <= *p && *p <= '9'; p++)
        x = x * 10 + *p - '0';
    *val = sign * x;
    return 0;
}

int cmdline_get_bool(const char *key, bool *val)
{
    const char *v, *vend;
    if (!cmdline_search(key, &v, &vend)) return -ENOENT;
    if (v == vend) return *val = true, 0;
    if (prefix_cmp(v, "true", vend - v))
        *val = true;
    else if (prefix_cmp(v, "false", vend - v))
        *val = false;
    else
        return -EINVAL;
    return 0;
}

char *cmdline_get_str_or(const char *key, char *def)
{
    char *str;
    if (cmdline_get_str(key, &str) < 0) return def;
    return str;
}

uint64_t cmdline_get_u64_or(const char *key, uint64_t def)
{
    uint64_t val;
    if (cmdline_get_u64(key, &val) < 0) return def;
    return val;
}

bool cmdline_get_bool_or(const char *key, bool def)
{
    bool val;
    if (cmdline_get_bool(key, &val) < 0) return def;
    return val;
}

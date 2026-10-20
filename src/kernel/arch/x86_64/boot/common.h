// clang-format off

// fetch qword
#define fq(x) ({ \
    addr_t __res;                 \
    __asm__ ("movabs $" #x ", %0" \
        : "=r"(__res)); __res;    \
})

// physical address of a qword var
#define pq(x) (fq(x) - __vbase)

#define align_up(x, y) ((y) * ((x + y - 1) / y))
#define align_dn(x, y) ((y) * (x / y))

typedef void *(getpage_t)();

extern __bcode void __common64(long magic, long info, getpage_t *getpage,
                               addr_t offset);

static inline __bcode void *memcpy(void *dest, const void *src, size_t n)
{
    char *p = dest;
    while (n--)
        *p++ = *(char *)src++;
    return dest;
}

static inline __bcode size_t strlen(const char *str)
{
    size_t i = 0;

    while (str && *str++)
        i++;
    return i;
}

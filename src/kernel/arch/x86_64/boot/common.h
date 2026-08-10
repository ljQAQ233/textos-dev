// fetch qword
#define fq(x) ({ \
    addr_t __res;                 \
    __asm__ ("movabs $" #x ", %0" \
        : "=r"(__res)); __res;    \
})

// physical address of a qword var
#define pq(x) (fq(x) - __vbase)

#define lbase fq(__lbase)
#define vbase fq(__vbase)
#define lend  fq(__lend)

#define align_up(x, y) ((y) * ((x + y - 1) / y))
#define align_dn(x, y) ((y) * (x / y))

typedef void *(getpage_t)();

extern __bcode void __common64(long magic, long info, getpage_t *getpage);

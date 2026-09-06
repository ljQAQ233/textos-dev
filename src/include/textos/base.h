#ifndef __BASE_H__
#define __BASE_H__

#define _ofp    __attribute__((optimize("omit-frame-pointer")))
#define _packed __attribute__((packed))

#define STATIC_ASSERT(expr, string) _Static_assert(expr, string)

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MIN(A, B) (((A) < (B)) ? (A) : (B))
#define ABS(X)    (((X) > 0) ? (X) : (-(X)))

#define OFFSET(ptr, offset) ((void *)(ptr) + (offset))
#define offsetof(type, m)   ((size_t)&((type *)0)->m)

#define CR(record, type, member) \
    ((type *)((void *)(record) - (void *)&((type *)((void *)0))->member))

#define DIV_ROUND_UP(num, divisor)   (((num) + ((divisor) - 1)) / (divisor))
#define DIV_ROUND_DOWN(num, divisor) ((num) / (divisor))

#define SIGN_16(A, B) ((A) | (B << 8))

#define SIGN_32(A, B, C, D) (SIGN_16(A, B) | (SIGN_16(C, D) << 16))

#define SIGN_64(A, B, C, D, E, F, G, H) \
    (SIGN_32(A, B, C, D) | ((u64)SIGN_32(E, F, G, H) << 32))

#define true  (1 == 1)
#define false (1 == 0)

#define NULL ((void *)0)
#define EOF  ((int)-1)

#define PAGE_SHIFT 12
#define PAGE_SIZ   (1 << PAGE_SHIFT)
#define PAGE_SIZE  (1 << PAGE_SHIFT)
#define PAGE_MASK  (PAGE_SIZ - 1)

#endif

#ifndef __BASE_H__
#define __BASE_H__

#define _ofp      __attribute__((optimize("omit-frame-pointer")))
#define _packed   __attribute__((packed))

/* Tools like MAX and MIN */
#define MAX(A,B) \
    (((A) > (B)) ? (A) : (B))
#define MIN(A,B) \
    (((A) < (B)) ? (A) : (B))

#define ABS(Num) \
    (((Num) > 0) ? (Num) : (-Num))

#define OFFSET(Ptr, Offset) \
    ((void *)(Ptr) + (Offset))

#define DIV_ROUND_UP(Num, Divisor) \
    (((Num) + ((Divisor) -  1)) / (Divisor))

#define DIV_ROUND_DOWN(Num, Divisor) \
    ((Num) / (Divisor))

#define SIGN_16(A, B) \
    ((A) | (B << 8))

#define SIGN_32(A, B, C, D) \
    (SIGN_16 (A, B) | (SIGN_16 (C, D) << 16))

#define SIGN_64(A, B, C, D, E, F, G, H) \
    (SIGN_32 (A, B, C, D) | ((u64)SIGN_32 (E, F, G, H) << 32))

#define CR(Record, Type, Member) \
    ((Type *)((void *)(Record) - (void *)&((Type *)((void *)0))->Member))

#define STATIC_ASSERT(Expr, String) \
        _Static_assert(Expr, String)

/* The Bool Type Value */
#define true     (1 == 1)
#define false    (1 == 0)

#define TRUE     true
#define FALSE    false

/* Enabled flags */
#define enable   true
#define disable  false

#define STATIC   static 
#define CONST    const  
#define VOID     void   

#define NULL     (VOID*)0

#define PAGE_SIZ 0x1000

#define  SIZE_4KB    0x00001000
#define  SIZE_2MB    0x00200000
#define  SIZE_1GB    0x40000000

#endif


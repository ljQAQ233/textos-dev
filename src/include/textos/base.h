#ifndef __BASE_H__
#define __BASE_H__

#define _packed   __attribute__((packed))

/* Tools like MAX and MIN */
#define MAX(A,B) \
    (A > B) ? A : B
#define MIN(A,B) \
    (A < B) ? A : B

#define ABS(Num) \
    (Num > 0) ? (Num) : (-Num)

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

#endif


#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define _STR(x)       #x
#define _CONCAT(x, y) x##y

// c23 standard
#undef unreachable
#define unreachable() assert(0)

#include "regs.h"

struct type;
struct field;
struct proto;

enum cls
{
    CLASS_INT,
    CLASS_ST,
    CLASS_PROTO,
};

#define def_printer(name) void name(FILE *o, struct type *t, void *ptr)
typedef def_printer((*printer_t));

struct type
{
    char *name;
    size_t size;
    enum cls cls;
    printer_t printer;
    union
    {
        struct
        {
            char conv;
        } INT;
        struct
        {
            struct field *field;
        } ST;
        struct
        {
            struct proto *proto;
        } PROTO;
    };
};

struct field
{
    char *name;
    size_t offset;
    struct type *type;
};

struct proto
{
    char *name;
    struct type *type;
};

//
// Basic data types
//
#define reftyb(N) (&_tyb_##N)

#define reftyb_auto(var, suffix)                 \
    _Generic((var),                              \
        void *: reftyb(ulx),                     \
        int: reftyb(i##suffix),                  \
        long: reftyb(l##suffix),                 \
        long long: reftyb(ll##suffix),           \
        unsigned: reftyb(u##suffix),             \
        unsigned long: reftyb(ul##suffix),       \
        unsigned long long: reftyb(ull##suffix), \
        default: reftyb(ulx))

//
// User-defined type support
//
#define refty(N)  (&_ty_##N)
#define reffun(N) (&_fun_##N)

#define refty_auto(var)             \
    _Generic((var),                 \
        struct stat: refty(stat),   \
        struct iovec: refty(iovec), \
        default: reftyb_auto(var, ))
#define refty_type(T)               \
    _Generic((T){0},                \
        struct stat: refty(stat),   \
        struct iovec: refty(iovec), \
        default: reftyb_auto((T)0, ))

//
// declarations
//
#define decltyb(N) extern struct type _tyb_##N;
#define declfun(N) extern struct type _fun_##N;
#define declty(N)  extern struct type _ty_##N;
#define declst(N)  extern struct field _st_##N[];

def_printer(int_printer);
def_printer(st_printer);
def_printer(proto_printer);

decltyb(i);
decltyb(l);
decltyb(ll);
decltyb(u);
decltyb(ul);
decltyb(ull);
decltyb(ix);
decltyb(lx);
decltyb(llx);
decltyb(ux);
decltyb(ulx);
decltyb(ullx);
decltyb(io);
decltyb(lo);
decltyb(llo);
decltyb(uo);
decltyb(ulo);
decltyb(ullo);

declty(stat);
declty(iovec);

#include "scnumb.h"

#define _(sc) declfun(sc)
SYSCALLS
XSYSCALLS
#undef _

#include "strace.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

void int_fmt(char *fmt, unsigned long long *val, int size, char conv)
{
    *fmt++ = '%';
    if (conv == 'x' || conv == 'o') {
        *fmt++ = '#';
    }
    // clang-format off
    switch (size) {
    case 4: break;
    case 1: *fmt++ = 'h';
    case 2: *fmt++ = 'h'; break;
    case 8: *fmt++ = 'l'; *fmt++ = 'l'; break;
    default:
        unreachable();
    }
    *fmt++ = conv;
    *fmt = '\0';
    if (conv == 'd') {
        switch (size) {
            case 1: *val = (signed char)*val; break;
            case 2: *val = (short)*val; break;
            case 4: *val = (int)*val; break;
            case 8: *val = (long long)*val; break;
            default: unreachable();
        }
    } else {
        switch (size) {
            case 1: *val = (unsigned char)*val; break;
            case 2: *val = (unsigned short)*val; break;
            case 4: *val = (unsigned int)*val; break;
            case 8: *val = (unsigned long long)*val; break;
            default: unreachable();
        }
    }
    // clang-format on
}

unsigned long long load_int(struct type *t, void *ptr)
{
    unsigned long long val = 0;
    memcpy(&val, ptr, t->size);
    return val;
}

def_printer(int_printer)
{
    unsigned long long val = (unsigned long long)v->ptr;
    char fmt[] = "%xxxx";
    memcpy(&val, v->ptr, t->size);
    int_fmt(fmt, &val, t->size, t->INT.conv);
    fprintf(o, fmt, val);
}

#include <ctype.h>

char esc(char c)
{
    switch (c) {
    case '\a':
        return 'a';
    case '\b':
        return 'b';
    case '\t':
        return 't';
    case '\n':
        return 'n';
    case '\v':
        return 'v';
    case '\f':
        return 'f';
    case '\r':
        return 'r';
    case '\\':
        return '\\';
    case '"':
        return '"';
    default:
        return c;
    }
}

// size: how many bytes in data need to be handled
int str_fmt(FILE *o, char *data, int size)
{
    for (int i = 0; i < size; i++) {
        unsigned char c = data[i];
        if (isprint(c) && c != '\\') {
            if (fputc(c, o) < 0) return -1;
        } else if (c == '\a' || c == '\b' || c == '\t' || c == '\n' ||
                   c == '\v' || c == '\f' || c == '\r' || c == '\\' ||
                   c == '"') {
            if (fputc('\\', o) < 0 || fputc(esc(c), o) < 0) return -1;
        } else {
            if (fprintf(o, "\\%o", c) < 0) return -1;
        }
    }
    return 0;
}

// Print the bytes behind a value. `nul` selects NUL-terminated vs bounded.
static void print_bytes(FILE *o, struct value *v)
{
    fprintf(o, "\"");
    str_fmt(o, v->ptr, v->len);
    fprintf(o, "\"");
    if (v->is_truncated) fprintf(o, "...");
}

def_printer(str_printer)
{
    print_bytes(o, v);
}

def_printer(buf_printer)
{
    print_bytes(o, v);
}

def_printer(st_printer)
{
    char *data = v->ptr;
    fprintf(o, "{ ");
    for (struct field *sub = t->ST.field; sub->name; sub++) {
        struct value fv;
        // scope resolves a bounded buffer's length from its sibling field
        if (sub->type->cls == CLASS_BUF && sub[1].name)
            fv.len = (long)load_int(sub[1].type, data + sub[1].offset);
        fprintf(o, ".%s = ", sub->name);
        load_data(sub->type, (unsigned long *)(data + sub->offset), &fv);
        sub->type->printer(o, sub->type, &fv);
        if (sub[1].name) fprintf(o, ", ");
    }
    fprintf(o, " }");
}

def_printer(arr_printer)
{
    struct type *et = t->ARR.elem_type;
    fprintf(o, "{ ");
    for (int i = 0; i < v->len; i++) {
        struct value av = {v->ptr + i * et->size, 0, -1};
        et->printer(o, et, &av);
        if (i != v->len - 1) fprintf(o, ", ");
    }
    fprintf(o, " }");
}

def_printer(proto_printer)
{
    struct proto *proto = t->PROTO.proto;
    struct proto *param = proto + 1;
    struct regs *regs = (struct regs *)v->ptr;
    fprintf(o, "%s(", t->name);
    for (int i = 0; param[i].name; i++) {
        struct value av;
        // scope resolves a bounded buffer's length from the next argument
        if ((param[i].type->cls == CLASS_BUF ||
             param[i].type->cls == CLASS_ARR) &&
            param[i + 1].name)
            av.len = (long)regs->arg[i + 1];
        load_data(param[i].type, &regs->arg[i], &av);
        param[i].type->printer(o, param[i].type, &av);
        if (param[i + 1].name) fprintf(o, ", ");
    }
    fprintf(o, ") = ");
    struct value rv = {&regs->ret, -1};
    proto->type->printer(o, proto->type, &rv);
    fprintf(o, "\n");
}

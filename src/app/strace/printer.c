#include "strace.h"
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

def_printer(int_printer)
{
    unsigned long long val = 0;
    char fmt[] = "%xxxx";
    memcpy(&val, ptr, t->size);
    int_fmt(fmt, &val, t->size, t->INT.conv);
    fprintf(o, fmt, val);
}

#include <ctype.h>

#define MAX_ELEM_STR 32

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
int str_fmt(FILE *o, char data[sizeof(long)], int size)
{
    int r = 0;
    for (int i = 0; i < sizeof(long); i++) {
        char c = data[i];
        if (size == -1 && !c) return 1;
        if (isprint(c) && c != '\\') {
            if (fputc(c, o) < 0) break;
        } else if (c == '\a' || c == '\b' || c == '\t' || c == '\n' ||
                   c == '\v' || c == '\f' || c == '\r' || c == '\\' ||
                   c == '"') {
            if (fputc('\\', o) < 0 || fputc(esc(c), o) < 0) break;
        } else {
            if (fprintf(o, "\\%o", c) < 0) break;
        }
    }
    return 0;
}

def_printer(str_printer)
{
    fprintf(o, "\"");
    char data[sizeof(long)];
    for (int i = 0; i < MAX_ELEM_STR; i += sizeof(long)) {
        peek_data(pid, *(unsigned long *)ptr + i, data, sizeof(long));
        if (str_fmt(o, data, -1) < 0) break;
    }
    fprintf(o, "\"");
}

def_printer(st_printer)
{
    char data[t->size * 2];
    peek_data(pid, *(unsigned long *)ptr, data, t->size);
    fprintf(o, "{ ");
    for (struct field *sub = t->ST.field; sub->name; sub++) {
        fprintf(o, ".%s = ", sub->name);
        sub->type->printer(o, sub->type, data + sub->offset);
        if (sub[1].name) fprintf(o, ", ");
    }
    fprintf(o, " }");
}

def_printer(proto_printer)
{
    struct proto *proto = t->PROTO.proto;
    struct proto *param = proto + 1;
    struct regs *regs = (struct regs *)ptr;
    fprintf(o, "%s(", t->name);
    for (int r = 0; param[r].name; r++) {
        param[r].type->printer(o, param[r].type, &regs->arg[r]);
        if (param[r + 1].name) fprintf(o, ", ");
    }
    fprintf(o, ") = ");
    proto->type->printer(o, proto->type, &regs->ret);
    fprintf(o, "\n");
}

#include "strace.h"
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

def_printer(st_printer)
{
    fprintf(o, "{ ");
    for (struct field *sub = t->ST.field; sub->name; sub++) {
        fprintf(o, ".%s = ", sub->name);
        sub->type->printer(o, sub->type, ptr + sub->offset);
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

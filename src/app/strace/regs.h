#pragma once

#include <sys/user.h>
#include <unistd.h>

struct regs
{
    unsigned long nr;
    unsigned long ret;
    union
    {
        struct
        {
            unsigned long a1;
            unsigned long a2;
            unsigned long a3;
            unsigned long a4;
            unsigned long a5;
            unsigned long a6;
        };
        unsigned long arg[6];
    };
};

void collect_args(struct regs *r, struct user_regs_struct *ur,
                  struct user_regs_struct *uro);

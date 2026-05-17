#pragma once

#include <sys/user.h>
#include <unistd.h>

struct regs
{
    size_t nr;
    size_t ret;
    union
    {
        struct
        {
            size_t a1;
            size_t a2;
            size_t a3;
            size_t a4;
            size_t a5;
            size_t a6;
        };
        size_t arg[6];
    };
};

void collect_args(struct regs *r, struct user_regs_struct *ur,
                  struct user_regs_struct *uro);

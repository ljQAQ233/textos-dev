#pragma once

#ifndef __MYCPU_H__
#error "mycpu.h cannot be included directly!"
#endif

#define mycpu_id()             \
    ({                         \
        int myid = 0;          \
        __asm__ volatile(      \
            "movl %%gs:%1, %0" \
            : "=r"(myid)       \
            : "m"(cpu_id));    \
        myid;                  \
    })

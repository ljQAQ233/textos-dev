#ifndef __MYCPU_H__
#define __MYCPU_H__

#define MYCPU_DEFINE(type, name) \
    __attribute__((section(".data.mycpu"))) \
    __typeof__(type) name
#define MYCPU_DECLARE(type, name) \
    extern MYCPU_DEFINE(type, name)

MYCPU_DECLARE(int, cpu_id);
MYCPU_DECLARE(addr_t, cpu_kstk);

extern addr_t *__mycpu_ptrs;

#define __get_cpu_varptr(base, name)       \
    ({                                     \
        addr_t __ptr = (addr_t)(&(name));  \
        (__typeof__(name)*)(__ptr + base); \
    })

#define get_cpu_base(cpu) __mycpu_ptrs[cpu]
#define get_cpu_var(cpu, name) *(__get_cpu_varptr(get_cpu_base(cpu), name))
#define get_cpu_varptr(cpu, name) (__get_cpu_varptr(get_cpu_base(cpu), name))

#include <mycpu.h>

#define mycpu_var(name) get_cpu_var(mycpu_id(), name)
#define mycpu_varptr(name) get_cpu_varptr(mycpu_id(), name)

#endif

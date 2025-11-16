#ifndef __AP_H__
#define __AP_H__

static inline int cpu_count()
{
    extern int __cpu_count;
    return __cpu_count;
}

static inline int cpu_started()
{
    extern int __cpu_started;
    return __cpu_started;
}

#endif

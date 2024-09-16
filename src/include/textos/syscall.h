#ifndef __SYSCALL_H__
#define __SYSCALL_H__

enum
{
    SYSCALL_READ,
    SYSCALL_WRITE,
    SYSCALL_CLOSE,
    SYSCALL_FORK,
    SYSCALL_TEST,
    SYSCALL_MAX,
} nr_syscall;

#endif

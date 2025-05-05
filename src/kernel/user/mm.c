#include <textos/syscall.h>
#include <textos/mm/mman.h>

RETVAL(void *) sys_mmap(void *addr, size_t len, int prot, int flgs, int fd, size_t off)
{
    return (RETVAL(void *))mmap(addr, len, prot, flgs, fd, off);
}

RETVAL(int) sys_mprotect(void *addr, size_t len, int prot)
{
    return mprotect(addr, len, prot);
}

RETVAL(int) sys_munmap(void *addr, size_t len)
{
    return munmap(addr, len);
}
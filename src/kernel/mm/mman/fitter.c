
#include <textos/task.h>
#include <textos/mm/mman.h>

addr_t vmm_fitaddr(addr_t addr, size_t num)
{
    task_t *tsk = task_current();
    addr_t res = tsk->mmap;
    num += 0x1; // skip some pages
    tsk->mmap += num * PAGE_SIZ;
    return res;
}

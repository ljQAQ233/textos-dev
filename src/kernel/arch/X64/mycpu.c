#include <cpu.h>
#include <textos/mycpu.h>

void arch_mycpu_init()
{
    addr_t addr = get_cpu_base(0);
    write_msr(MSR_GS_BASE, addr);
}

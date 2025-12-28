// i387 fpu

#include <cpu.h>
#include <irq.h>
#include <intr.h>
#include <textos/mm.h>
#include <textos/task.h>
#include <textos/panic.h>
#include <textos/dev/fpu/i387.h>

static task_t *last;

void fpu_enable(task_t *tsk)
{
    if (tsk->fpu)
        frstor(tsk->fpu);
    else {
        size_t siz = sizeof(i387_stat_t);
        tsk->fpu = malloc(siz);

        fnclex(); // 清除异常
        fninit(); // 初始化
    }
}

void fpu_disable()
{
    write_cr0(read_cr0() | CR0_TS);
}

void fpu_savest(task_t *tsk)
{
    fnsave(tsk->fpu);
}

// INT #NM
__INTR_HANDLER(fpu_handler)
{
    task_t *tsk = task_current();

    // fpu 启动!
    size_t cr0 = read_cr0();
    if (cr0 & CR0_TS)
    {
        write_cr0(cr0 &~ CR0_TS); // clts
        if (tsk != last) {
            if (last)
                fpu_savest(last);
            fpu_enable(tsk);
            last = tsk;
        }
    }
}

// INT #MF
__INTR_HANDLER(math_handler)
{
    PANIC("fpu exception occurred!!!\n");
}

bool fpu_detect()
{
    u32 eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    return edx & 1;
}

void fpu_init()
{
    // todo : emu
    if (!fpu_detect())
        PANIC("fpu not available\n");

    // native mode
    write_cr0(read_cr0() | CR0_NE);
    intr_register(INT_MF, math_handler);
    intr_register(INT_NM, fpu_handler);
    DEBUGK(K_INFO, "fpu initialized!\n");
}


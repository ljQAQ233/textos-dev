#include <textos/textos.h>
#include <textos/debug.h>

#include <boot.h>

extern void kernel_main();

extern void __kstack_init ();

extern void __video_pre (vconfig_t *v);
extern void __mm_pre (mconfig_t *m);

static bconfig_t bconfig;

#include <string.h>

void kernel_init (bconfig_t *config)
{
    dprintk_set(K_ALL & ~K_SYNC);
    DEBUGK(K_INIT, "kernel_init(%p)\n", config);
    
    memcpy (&bconfig, config, sizeof(bconfig_t));

    /*
       直接换栈会有丢失数据的风险, 比如 config 在 换栈之后会失效,
       这是由于编译器在调用它的时候把 rdi 寄存器送入了栈中
    */
    __kstack_init();

    __video_pre (&bconfig.video);
    __mm_pre (&bconfig.memory);

    kernel_main();
}


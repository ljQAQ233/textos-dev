/*
  This is for disabling 8259 interrupt controller.
*/

#include <io.h>

#define MPIC_DATA 0x21
#define SPIC_DATA 0xA1

void pic_disable ()
{
    /* Mask all interrupts in 8259 */
    outb (SPIC_DATA, 0xFF);
    outb (MPIC_DATA, 0xFF);
}

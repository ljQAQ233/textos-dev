#ifndef __INTR_H__
#define __INTR_H__

#include <cpu.h>

typedef void (*ihandler_t)(
        u8 vector,
        u64 errcode,
        intr_frame_t *frame
        );

#define __INTR_HANDLER(name) void name(u8 vector, u64 errcode, intr_frame_t *frame)

void intr_register (u8 vector, ihandler_t handler);

// the common handler

extern __INTR_HANDLER (intr_common);

// interrupt exit
extern void intr_exit ();

#endif

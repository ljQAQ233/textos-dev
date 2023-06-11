#ifndef __CPU_H__
#define __CPU_H__

void read_get (void *gdtr);

void load_gdt (void *gdtr);

void reload_segs (u64 ss, u64 cs);

#endif


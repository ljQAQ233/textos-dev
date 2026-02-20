#ifndef __IO_H__
#define __IO_H__

void outb (u16 port, u8 data);

void outw (u16 port, u16 data);

void outdw (u16 port, u32 data);

u8 inb (u16 port);

u16 inw (u16 port);

u32 indw (u16 poet);

#endif

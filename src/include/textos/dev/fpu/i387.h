#pragma once

typedef struct _packed {
  u32 cwd; // 控制字 bit0 ~ bit15
  u32 swd; // 状态字
  u32 twd;
  u32 fip;
  u32 fcs;
  u32 foo;
  u32 fos;
  u8  regs[80]; // 8 * 寄存器 (80bit)
} i387_stat_t;

void fninit();
void fnclex();
void fnstcw(u16 *ctrl);
void fldcw(u16 *ctrl);
void fnstsw(u16 *sreg);
void fnsave(void *stat);
void frstor(void *stat);

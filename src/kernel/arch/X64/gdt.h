#ifndef __GDT_H__
#define __GDT_H__

#define KERN_CODE_SEG   1
#define KERN_DATA_SEG   2
#define USER_CODE32_SEG 3
#define USER_DATA_SEG   4
#define USER_CODE_SEG   5

#define TSS_LOAD_SEG    6

void tss_set (u64 rsp);

#endif

#ifndef __DEBUG_H__
#define __DEBUG_H__

void dprintk(int lv, const char *format, ...);

#define K_NONE  0         // none
#define K_LOGK  (1 << 0)  // log
#define K_WARN  (1 << 1)  // warn
#define K_ERRO  (1 << 2)  // error
#define K_SYNC  (1 << 3)  // sync to console
#define K_INIT  (1 << 4)  // initializer

#define K_MM    (1 << 5)  // memory manager
#define K_TASK  (1 << 6)  // task manager
#define K_DEV   (1 << 7)  // device
#define K_FS    (1 << 8)  // file system
#define K_PIC   (1 << 9)  // pic / apic
#define K_KBD   (1 << 10) // keyboard

#define K_ALL   ((unsigned)-1)

int dprintk_set(int mask);

void debugk(int lv, const char *file, const int line, const char *format, ...);

#if !defined(kconf_release)
    #define DEBUGK(lv, format, ARGS...) \
            debugk(lv, __FILE__, __LINE__, format, ##ARGS)
#else
    #define DEBUGK(format, ARGS...) 
#endif

#endif

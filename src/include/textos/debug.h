#ifndef __DEBUG_H__
#define __DEBUG_H__

void debugk (
        const char *file,
        const u64  line,
        const char *format,
        ...
        );

#ifdef __TEXTOS_DEBUG
    #define DEBUGK(format, ARGS...) \
            debugk (__FILE__, __LINE__, format, ##ARGS)
#else
    #define DEBUGK(format, ARGS...) 
#endif

#endif

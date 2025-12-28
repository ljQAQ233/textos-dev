#ifndef __DEBUG_H__
#define __DEBUG_H__

void dprintk(const char *format, ...);

#define K_FATAL 0 // Fatal error, program cannot continue
#define K_ERROR 1 // Error, something went wrong
#define K_WARN  2 // Warning, unusual but recoverable
#define K_INFO  3 // Info, normal but important messages
#define K_DEBUG 4 // Debug, for debugging purposes
#define K_TRACE 5 // Trace, very detailed debug information

#define K_LVMSK 0xff     // level mask
#define K_CONT  (1 << 8) // it's a continuation
#define K_WORDY (1 << 9) // print file and line number

#ifndef K_LEVEL
    #define K_LEVEL (K_DEBUG | K_WORDY)
#endif

#define _STR(x) #x
#define STR(x)  _STR(x)

#ifndef CONFIG_RELEASE
    #define DEBUGK(lv, format, ARGS...)                                 \
        do {                                                            \
            if (((lv) & K_LVMSK) <= K_LEVEL) {                          \
                if ((((lv) | K_LEVEL) & K_WORDY)) {                     \
                    dprintk("[" __FILE__ ":" STR(__LINE__) "] ", \
                            ##ARGS);                                    \
                }                                                       \
                if ((lv) & K_CONT) dprintk(" | ");                      \
                dprintk(format, ##ARGS);                                \
            }                                                           \
        } while (0)
#else
    #define DEBUGK(lv, format, ARGS...)
#endif

#endif

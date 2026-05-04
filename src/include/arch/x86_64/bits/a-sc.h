/*
 * actually `long` already keeps the same size of registers
 * here we give a more obvious definition - type used by syscall
 */
typedef long long __sc_type;

#define __syscall(num, a1, a2, a3, a4, a5, a6)                         \
    ({                                                                 \
        register __sc_type __r0 __asm__("rax") = (__sc_type)(num);     \
        register __sc_type __r1 __asm__("rdi") = (__sc_type)(a1);      \
        register __sc_type __r2 __asm__("rsi") = (__sc_type)(a2);      \
        register __sc_type __r3 __asm__("rdx") = (__sc_type)(a3);      \
        register __sc_type __r4 __asm__("r10") = (__sc_type)(a4);      \
        register __sc_type __r5 __asm__("r8") = (__sc_type)(a5);       \
        register __sc_type __r6 __asm__("r9") = (__sc_type)(a6);       \
                                                                       \
        __asm__ volatile("syscall"                                     \
                         : "+r"(__r0)                                  \
                         : "r"(__r1), "r"(__r2), "r"(__r3), "r"(__r4), \
                           "r"(__r5), "r"(__r6)                        \
                         : "memory", "rcx", "r11");                    \
        __r0;                                                          \
    })

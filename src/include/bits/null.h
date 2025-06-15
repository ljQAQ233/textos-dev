/*
 * NULL may be defined in many headfiles of libc
 */
#if __cplusplus >= 201103L
  #define NULL nullptr
#elif defined(__cplusplus)
  #define NULL 0L
#else
  #define NULL ((void*)0)
#endif
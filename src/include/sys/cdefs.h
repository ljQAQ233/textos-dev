#ifndef	_SYS_CDEFS_H
#define	_SYS_CDEFS_H

#ifdef __cplusplus
  #define __BEGIN_DECLS extern "C" {
  #define __END_DECLS }
#else
  #define __BEGIN_DECLS
  #define __END_DECLS
#endif

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define _Noreturn _Noreturn
#else
  #define _Noreturn __attribute__((noreturn))
#endif

#endif
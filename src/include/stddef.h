#ifndef _STDDEF_H
#define _STDDEF_H

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <bits/null.h>

#define __NEED_size_t
#define __NEED_wchar_t
#define __NEED_ptrdiff_t

#if __STDC_VERSION__ >= 201112L || __cplusplus >= 201103L
  #define __NEED_max_align_t
#endif

#include <bits/alltypes.h>

#define offsetof(type, member) ((size_t)((char *)&(((type *)0)->member) - (char *)0))

__END_DECLS

#endif
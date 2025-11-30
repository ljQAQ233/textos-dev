#ifndef __NOOPT_H__
#define __NOOPT_H__

extern void __noopt_inval();
extern void __noopt_perm();

#define noopt (void *)__noopt_inval
#define noopt_perm (void *)__noopt_perm

#endif

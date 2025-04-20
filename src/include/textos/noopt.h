#ifndef __NOOPT_H__
#define __NOOPT_H__

extern void __noopt_inval();

#define noopt (void *)__noopt_inval

#endif
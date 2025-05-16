#ifndef __UTSNAME_H__
#define __UTSNAME_H__

typedef struct utsname
{
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
} utsname_t;

#ifdef __TEXT_OS__

#define UTS_SYSNAME  "textos"
#define UTS_NODENAME "textos"

#ifndef UTS_RELEASE
  #define UTS_RELEASE "unknown"
#endif

#ifndef UTS_VERSION
  #define UTS_VERSION "unknown"
#endif

#define UTS_MACHINE  ARCH_STRING

#endif

#endif

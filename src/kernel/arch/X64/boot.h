#ifndef __BOOT_H__
#define __BOOT_H__

typedef struct {
  u64 hor;
  u64 ver;
  u64 fb;     // frame buffer
  u64 fb_siz; // frame buffer size
} vconfig_t;

typedef struct {
    u64       magic;
    vconfig_t video;
} bconfig_t;

#endif

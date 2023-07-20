#ifndef __BOOT_H__
#define __BOOT_H__

typedef struct {
  u64 hor;
  u64 ver;
  u64 fb;     // frame buffer
  u64 fb_siz; // frame buffer size
} vconfig_t;

typedef struct {
  void  *maps;
  u64   mapsiz;
  u64   mapcount;
  u64   mapkey;
  u64   descsiz;
  u32   descver;
} mapinfo_t;

typedef struct {
  u8    va;   // valid
  u64   msiz; // memory siz
  void *phy;
  void *vrt;
} kpgs_t;

typedef struct {
  void   *map;  // mapinfo_t;
  void   *kpgs; // kpgs_t;
} mconfig_t;

typedef struct {
    u64       magic;
    vconfig_t video;
    mconfig_t memory;
    void      *acpi;
} bconfig_t;

bconfig_t *bconfig_get ();

#endif

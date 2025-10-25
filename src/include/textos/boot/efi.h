#pragma once

#define TEXTOS_BOOT_MAGIC SIGN_64('T', 'E', 'X', 'T', 'O', 'S', 'B', 'T')

typedef struct
{
    u64 hor;
    u64 ver;
    u64 fb;     // frame buffer
    u64 fb_siz; // frame buffer size
} vconfig_t;

typedef struct
{
    u8 va;
    u64 cnt;
    void *ptr;
} balloc_t;

typedef struct
{
    void *maps;
    u64 mapsiz;
    u64 mapcount;
    u64 mapkey;
    u64 descsiz;
    u32 descver;
} mapinfo_t;

typedef struct
{
    u8 va;    // valid
    u64 msiz; // memory siz
    void *phy;
    void *vrt;
} kpgs_t;

typedef struct
{
    balloc_t balloc[16];
    void *map;  // mapinfo_t;
    void *kpgs; // kpgs_t;
} mconfig_t;

typedef struct
{
    u64 magic;
    vconfig_t video;
    mconfig_t memory;
    void *acpi;
    void *runtime;
} bconfig_t;

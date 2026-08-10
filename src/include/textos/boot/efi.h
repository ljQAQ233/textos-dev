#pragma once

#define TEXTOS_BOOT_MAGIC 0x5442534f54584554

typedef struct
{
    void *map;
    unsigned long long mapsiz;
    unsigned long long mapkey;
    unsigned long long desccnt;
    unsigned long long descsiz;
    unsigned long long descver;
} mapinfo_t;

typedef struct
{
    unsigned long long magic;
    unsigned long long hor;
    unsigned long long ver;
    unsigned long long fb;     // frame buffer
    unsigned long long fb_siz; // frame buffer size
    void *mapinfo;
    void *acpi;
    void *runtime;
    unsigned long long load_base;
    unsigned long long load_size;
    unsigned long long phy_entry;
} bconfig_t;

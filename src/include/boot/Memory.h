#ifndef __MEMORY_H__
#define __MEMORY_H__

typedef struct {
    EFI_MEMORY_DESCRIPTOR *Maps;
    UINTN                 MapSiz;
    UINTN                 MapCount;
    UINTN                 MapKey;
    UINTN                 DescSiz;
    UINT32                DescVersion;
} MAP_INFO;

EFI_STATUS
MemoryGetMap (
  MAP_INFO *Info
  );

#endif

#include <textos/uefi.h>

#define MEMTYPE(type) [type] = #type

static const char *memtype[] = {
    MEMTYPE(EfiReservedMemoryType),
    MEMTYPE(EfiLoaderCode),
    MEMTYPE(EfiLoaderData),
    MEMTYPE(EfiBootServicesCode),
    MEMTYPE(EfiBootServicesData),
    MEMTYPE(EfiRuntimeServicesCode),
    MEMTYPE(EfiRuntimeServicesData),
    MEMTYPE(EfiConventionalMemory),
    MEMTYPE(EfiUnusableMemory),
    MEMTYPE(EfiACPIReclaimMemory),
    MEMTYPE(EfiACPIMemoryNVS),
    MEMTYPE(EfiMemoryMappedIO),
    MEMTYPE(EfiMemoryMappedIOPortSpace),
    MEMTYPE(EfiPalCode),
    MEMTYPE(EfiPersistentMemory),
    MEMTYPE(EfiMaxMemoryType),
};

static const char *memnone = "InvalidMemoryType"; 

const char *get_uefi_mtstr(int x)
{
    if (x >= EfiMaxMemoryType)
        return NULL;
    return memtype[x];
}

const char *get_uefi_mtstrv(int x)
{
    if (x >= EfiMaxMemoryType)
        return memnone;
    return memtype[x];
}


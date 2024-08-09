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

#define NR_WARNCODE 5
#define NR_ERROCODE 33

static const char *stat[] = {
    "Success",                      //  RETURN_SUCCESS                = 0
    "Warning Unknown Glyph",        //  RETURN_WARN_UNKNOWN_GLYPH     = 1
    "Warning Delete Failure",       //  RETURN_WARN_DELETE_FAILURE    = 2
    "Warning Write Failure",        //  RETURN_WARN_WRITE_FAILURE     = 3
    "Warning Buffer Too Small",     //  RETURN_WARN_BUFFER_TOO_SMALL  = 4
    "Warning Stale Data",           //  RETURN_WARN_STALE_DATA        = 5
    "Load Error",                   //  RETURN_LOAD_ERROR             = 1  | MAX_BIT
    "Invalid Parameter",            //  RETURN_INVALID_PARAMETER      = 2  | MAX_BIT
    "Unsupported",                  //  RETURN_UNSUPPORTED            = 3  | MAX_BIT
    "Bad Buffer Size",              //  RETURN_BAD_BUFFER_SIZE        = 4  | MAX_BIT
    "Buffer Too Small",             //  RETURN_BUFFER_TOO_SMALL,      = 5  | MAX_BIT
    "Not Ready",                    //  RETURN_NOT_READY              = 6  | MAX_BIT
    "Device Error",                 //  RETURN_DEVICE_ERROR           = 7  | MAX_BIT
    "Write Protected",              //  RETURN_WRITE_PROTECTED        = 8  | MAX_BIT
    "Out of Resources",             //  RETURN_OUT_OF_RESOURCES       = 9  | MAX_BIT
    "Volume Corrupt",               //  RETURN_VOLUME_CORRUPTED       = 10 | MAX_BIT
    "Volume Full",                  //  RETURN_VOLUME_FULL            = 11 | MAX_BIT
    "No Media",                     //  RETURN_NO_MEDIA               = 12 | MAX_BIT
    "Media changed",                //  RETURN_MEDIA_CHANGED          = 13 | MAX_BIT
    "Not Found",                    //  RETURN_NOT_FOUND              = 14 | MAX_BIT
    "Access Denied",                //  RETURN_ACCESS_DENIED          = 15 | MAX_BIT
    "No Response",                  //  RETURN_NO_RESPONSE            = 16 | MAX_BIT
    "No mapping",                   //  RETURN_NO_MAPPING             = 17 | MAX_BIT
    "Time out",                     //  RETURN_TIMEOUT                = 18 | MAX_BIT
    "Not started",                  //  RETURN_NOT_STARTED            = 19 | MAX_BIT
    "Already started",              //  RETURN_ALREADY_STARTED        = 20 | MAX_BIT
    "Aborted",                      //  RETURN_ABORTED                = 21 | MAX_BIT
    "ICMP Error",                   //  RETURN_ICMP_ERROR             = 22 | MAX_BIT
    "TFTP Error",                   //  RETURN_TFTP_ERROR             = 23 | MAX_BIT
    "Protocol Error",               //  RETURN_PROTOCOL_ERROR         = 24 | MAX_BIT
    "Incompatible Version",         //  RETURN_INCOMPATIBLE_VERSION   = 25 | MAX_BIT
    "Security Violation",           //  RETURN_SECURITY_VIOLATION     = 26 | MAX_BIT
    "CRC Error",                    //  RETURN_CRC_ERROR              = 27 | MAX_BIT
    "End of Media",                 //  RETURN_END_OF_MEDIA           = 28 | MAX_BIT
    "Reserved (29)",                //  RESERVED                      = 29 | MAX_BIT
    "Reserved (30)",                //  RESERVED                      = 30 | MAX_BIT
    "End of File",                  //  RETURN_END_OF_FILE            = 31 | MAX_BIT
    "Invalid Language",             //  RETURN_INVALID_LANGUAGE       = 32 | MAX_BIT
    "Compromised Data"              //  RETURN_COMPROMISED_DATA       = 33 | MAX_BIT
};

static const char *statnone = "Invalid Status";

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

const char *get_uefi_statstr(EFI_STATUS x)
{
    const char *res = statnone;
    if (EFI_ERROR(x)) {
        x &= ~MAX_BIT;
        if (0 < x && x <= NR_ERROCODE)
            res = stat[x];
    } else {
        if (0 <= x && x <= NR_WARNCODE)
            res = stat[x];
    }

    return res;
}

#include <boot.h>
#include <string.h>
#include <textos/mm.h>
#include <textos/debug.h>
#include <textos/panic.h>

void __uefi_tovmm()
{
    static bool shot;
    if (shot)
        return;
    else
        shot = true;

    bconfig_t *bconfig = bconfig_get();
    mapinfo_t *info = bconfig->memory.map;

    addr_t vs = __uefi_misc;
    EFI_MEMORY_DESCRIPTOR *desc = OFFSET(info->maps, info->descsiz);
    for (int i = 1 ; i < info->mapcount ; i++) {
        if (desc->Attribute & EFI_MEMORY_RUNTIME) {
            desc->VirtualStart = vs;
            addr_t ps = desc->PhysicalStart;
            size_t num = desc->NumberOfPages;
            u16    mapflg = PE_P | PE_RW;    // TODO
            vmap_map(ps, vs, num, mapflg, MAP_4K);
            DEBUGK(K_INIT, "[#%02d] runtime region detected\n", i);

            vs += desc->NumberOfPages * PAGE_SIZ;
            if (vs >= __uefi_max)
                PANIC("remap uefi failed - out of resource\n");
        }
        desc = OFFSET(desc, info->descsiz);
    }

    EFI_RUNTIME_SERVICES *rt = bconfig->runtime;
    EFI_STATUS stat = rt->SetVirtualAddressMap (
        info->mapsiz,
        info->descsiz,
        info->descver,
        info->maps
    );
    if (EFI_ERROR(stat))
        PANIC("failed to SetVirtualAddressMap - %s\n", get_uefi_statstr(stat));
    else
        DEBUGK(K_INIT, "SetVirtualAddressMap() okay - %s\n", get_uefi_statstr(stat));
    
    void *newrt = malloc(sizeof(*rt));
    bconfig->runtime = memcpy(newrt, rt, sizeof(*rt));

    for (balloc_t *p = bconfig->memory.balloc ; p->va ; p++)
        pmm_freepages(p->ptr, p->cnt);
}


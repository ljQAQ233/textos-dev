#include <Uefi.h>

#include <Boot.h>
#include <Config.h>

INI_INFO Ini;

EFI_STATUS InitializeConfig ()
{
    ERR_RETS(IniLoad (CONFIG_PATH,&Ini));

    return EFI_SUCCESS;
}

EFI_STATUS ConfigGetStringChar16S (
        IN     CHAR8    *Key,
        IN     CHAR8    *Default,
           OUT CHAR16   **String
        )
{
    return IniGetStringChar16S (
            &Ini,
            Key,Default,
            String
            );
}

CHAR16 *ConfigGetStringChar16 (
        IN CHAR8    *Key,
        IN CHAR8    *Default
        )
{
    return IniGetStringChar16 (
            &Ini,
            Key,Default
            );
}

EFI_STATUS ConfigGetNumUint64S (
        IN     CHAR8    *Key,
        IN     UINT64   Default,
           OUT UINT64   *Number
        )
{
    return IniGetNumUint64S (
            &Ini,
            Key,Default,
            Number
            );
}

UINT64 ConfigGetNumUint64 (
        IN CHAR8    *Key,
        IN UINT64   Default
        )
{
    return IniGetNumUint64 (
            &Ini,
            Key,Default
            );
}


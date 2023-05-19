#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Boot.h>
#include <File.h>
#include <Ini.h>

enum {
  KEY,
  VAL,
  COMMENT,
} INI_CUR;

STATIC VOID IniParseLine (INI_INFO *Ini, CHAR8 *Line)
{
    CHAR8 *Ptr = Line;
    CHAR8 *KeyStart = Ptr,
          *ValueStart = 0;
    UINT64 KeyLen = 0,
           ValueLen = 0;
    UINT64 Cur = KEY;
    BOOLEAN InSentences = FALSE;
    while (*Ptr != '\n' && *Ptr != '\0')
    {
        if (Cur == COMMENT)
        {
            goto Summary;
        }
        switch (*Ptr) {
            case ' ':
                if (Cur == KEY && !InSentences)
                {
                    KeyStart++;
                }
                else if (Cur == VAL && !InSentences)
                {
                    ValueStart++;
                }
                break;

            case '=':
                InSentences = FALSE;
                Cur = VAL;
                ValueStart = Ptr + 1;
                break;

            /* Check if it is the sign of comments */
            case '#':
                Cur = COMMENT;
                break;
            case ';':
                Cur = COMMENT;
                break;

            default:
                InSentences = TRUE;
                if (Cur == KEY)
                {
                    KeyLen++;
                }
                else if (Cur == VAL)
                {
                    ValueLen++;
                }
            ;
        }
        Ptr++;
    }

Summary:
    if (KeyLen == 0)
    {
        return;
    }
    if (ValueLen == 0)
    {
        ValueStart = "NULL";
        ValueLen   = AsciiStrLen (ValueStart);
    }

    KV_TABLE *New = AllocateZeroPool (sizeof(KV_TABLE));

    New->Key = AllocateZeroPool (KeyLen + 1);
    AsciiStrnCpy (New->Key, KeyStart, KeyLen);
    New->Value = AllocateZeroPool (ValueLen + 1);
    AsciiStrnCpy (New->Value, ValueStart, ValueLen);
    DEBUG ((DEBUG_INFO ,"[INFO] Key : %a , Value : %a\n", New->Key, New->Value));

    New->Next = Ini->Kvs;
    Ini->Kvs  = New;

    Ini->Count++;
}

EFI_STATUS IniLoad (
        IN     CHAR16* Path,
           OUT INI_INFO *IniInfo
        )
{
    if (IniInfo == NULL || Path == NULL)
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Invlaid parameters,can't be NULL\n"));
        return EFI_INVALID_PARAMETER;
    }
    /* Basic init */
    IniInfo->Count = 0;
    IniInfo->Kvs   = NULL;

    EFI_FILE_PROTOCOL *File;
    ERR_RETS(FileOpen (Path, O_READ, &File));

    CHAR8 *IniRaw;
    ERR_RETS(FileAutoRead (File, (VOID**)&IniRaw, NULL));

    UINTN Len = AsciiStrLen (IniRaw);

    for (UINTN i = 0;i < Len;i++)
    {
        if (IniRaw[i] == '\n' || i == 0)
        {
            IniParseLine (IniInfo, &IniRaw[i + (i == 0 ? 0 : 1)]);
        }
    }

    FreePool (IniRaw);
    DEBUG ((DEBUG_INFO ,"[ OK ] Loaded all valid expressions\n"));

    return EFI_SUCCESS;
}

CHAR8 *IniGet (
        INI_INFO *Ini,
        CHAR8    *Key
        )
{
    KV_TABLE *Kv = Ini->Kvs;
    for (UINTN i = 0;i < Ini->Count && Kv != NULL;i++)
    {
        if (AsciiStrCmp (Kv->Key,Key) == 0)
        {
            return Kv->Value;
        }
        Kv = Kv->Next;
    }

    return NULL;
}

EFI_STATUS IniGetStringChar16S (
        IN     INI_INFO *Ini,
        IN     CHAR8    *Key,
        IN     CHAR8    *Default,
           OUT CHAR16   **String
        )
{
    EFI_STATUS Status = EFI_SUCCESS;

    CHAR8 *Value = IniGet (Ini, Key);
    if (Value == NULL || AsciiStrCmp (Value, "NULL") == 0)
    {
        Status = EFI_NOT_FOUND;
        Value = Default;
    }

    UINTN Len = AsciiStrLen (Value);
    *String = AllocateZeroPool ((Len + 1) * sizeof(CHAR16));
    if (*String == NULL)
    {
        return EFI_OUT_OF_RESOURCES;
    }
    AsciiStrToUnicodeStr (Value, *String);

    return Status;
}

CHAR16 *IniGetStringChar16 (
        IN INI_INFO *Ini,
        IN CHAR8    *Key,
        IN CHAR8    *Default
        )
{
    CHAR16 *String;

    if (EFI_ERROR (IniGetStringChar16S (Ini, Key, Default, &String)))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Unable to get string from key %a,use the default - %a\n", Key, Default));
    }
    else
    {
        DEBUG ((DEBUG_INFO ,"[ OK ] Get string from key %a - %s\n", Key, String));
    }
    
    return String;
}

EFI_STATUS IniGetNumUint64S (
        IN     INI_INFO *Ini,
        IN     CHAR8    *Key,
        IN     UINT64   Default,
           OUT UINT64   *Number
        )
{
    CHAR8 *Value = IniGet (Ini, Key);
    if (Value == NULL || AsciiStrCmp (Value, "NULL") == 0)
    {
        *Number = Default;
        return EFI_NOT_FOUND;
    }

    if (Value[0] == '0' && (Value[1] == 'x' || Value[2] == 'X'))
    {
        *Number = AsciiStrHexToUint64 (Value);
    }
    else
    {
        *Number = AsciiStrDecimalToUint64 (Value);
    }

    return EFI_SUCCESS;
}

UINT64 IniGetNumUint64 (
        IN INI_INFO *Ini,
        IN CHAR8    *Key,
        IN UINT64   Default
        )
{
    UINT64 Number = 0;

    if (EFI_ERROR (IniGetNumUint64S (Ini, Key, Default, &Number)))
    {
        DEBUG ((DEBUG_ERROR ,"[FAIL] Unable to get number from key %a,use the default - %llu\n", Key, Number));
    }
    else
    {
        DEBUG ((DEBUG_INFO ,"[ OK ] Get number from key %a - %llu\n", Key, Number));
    }
    
    return Number;
}


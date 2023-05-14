#include <Uefi.h>

/* Get power number of unsigned types X^Y*/
UINT64 PowerU64 (UINT64 X,UINTN Y)
{
    if (X == 0)
    {
        return 0;
    }

    if (Y == 0)
    {
        return 1;
    }

    UINT64 Result = X;
    while (--Y)
    {
        Result *= X;
    }

    return Result;
}


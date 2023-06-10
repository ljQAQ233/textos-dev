#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

typedef long long           INT64;
typedef unsigned long long  UINT64;

typedef int                 INT32;
typedef unsigned int        UINT32;

typedef short               INT16;
typedef unsigned short      UINT16;

typedef signed char         INT8;
typedef unsigned char       UINT8;

typedef INT64               INTN;
typedef UINT64              UINTN;

typedef char                CHAR8;
typedef unsigned short      CHAR16;

typedef unsigned char       BOOLEAN;

typedef UINT64  PHYSICAL_ADDRESS;
typedef UINT64  VIRTUAL_ADDRESS;

typedef long long           int64;
typedef unsigned long long  u64;

typedef int                 int32;
typedef unsigned int        u32;

typedef short               int16;
typedef unsigned short      u16;

typedef signed char         int8;
typedef unsigned char       u8;

typedef char                char8;
typedef unsigned short      char16;

enum {
  LEFT    = 1,
  SIGN    = 1 << 1,
  ZERO    = 1 << 2,
  SPECIAL = 1 << 3,
  SPACE   = 1 << 4,
};

#define IsDigit(Object) ('0' <= (char)Object && (char)Object <= '9')

int __Int (char *Ptr, int *Width)
{
    int i = 0;
    int l = 0;

    while (IsDigit (*Ptr)) {
        i = i * 10 + *Ptr++ - '0';
        l++;
    }

    *Width = i;

    return l;
}

#define TMP_BUFFER_SIZE 64

int64 __Number (char *Buffer, u64 Num, int Base, bool Upper)
{
    char *Letters = !Upper ? "0123456789abcdef" : "0123456789ABCDEF";

    int64 Siz = 0;
    char Tmp[TMP_BUFFER_SIZE];

    char *Ptr = Tmp;

    if (Num == 0) {
        *Ptr++ = '0';
        Siz++;
    } else {
        while (Num != 0) {
            *Ptr++ = Letters[Num % Base];
            Num /= Base;
            Siz++;
        }
    }

    Ptr--;
    int64 i = 0;
    while (i < Siz) {
        Buffer[i++] = *Ptr--;
    }
    Buffer[i] = '\0';

    return Siz;
}

int64 VSPrint (char *Buffer, char *Format, va_list Args)
{
    char *Ptr = Format;
    char *Out = Buffer;
    char Tmp[TMP_BUFFER_SIZE];

    int Flgs = 0;

    while (Ptr && *Ptr)
    {
        if (*Ptr != '%')
        {
            *Out++ = *Ptr++;
            continue;
        }
ParseFlgs:
        Ptr++;
        switch (*Ptr) {
            case '#': // 与 o,x或X 一起使用时,非零值前面会分别显示 0,0x或0X
                Flgs |= SPECIAL;
                goto ParseFlgs;
            case '0': // 在指定填充的数字左边放置0,而不是空格
                if (Flgs & ZERO) {
                    break; // 再有就是宽度
                }
                Flgs |= ZERO;
                goto ParseFlgs;
            case '-': // 在给定的字段宽度内左对齐,默认是右对齐
                Flgs |= LEFT;
                goto ParseFlgs;
            case ' ': // 如果没有写入任何符号,则在该值前面填空格
                Flgs |= SPACE;
                goto ParseFlgs;
            case '+': // 如果是正数,则在最前面加一个正号
                Flgs |= SIGN;
                goto ParseFlgs;
            default:
                break;
        }
        Ptr--;

        int Offset = 0;

        int Radix = 10;
        int Length = 0;
        bool Signed = false;
        int Width = 0;
        bool UpperCase = false;
ParseArgs:
        Ptr++;
        switch (*Ptr)
        {
            case '%':
                *Out++ = '%';
                break;
            case 'l':
            case 'L':
                Length = 1;
                if (*(Ptr+1) == 'l' || *(Ptr+1) == 'L') {
                    Ptr++;
                    Length = 2;
                }
                goto ParseArgs;

            case 'X':
                UpperCase = true;
            case 'x':
                Radix = 16;
                break;
            case 'o':
                Radix = 8;
                break;
            case 'd':
            case 'i':
                Signed = true;
            case 'u':
                Radix = 10;
                break;
            case 'c':
                /* Includes the char */
                if (Width > 1)
                    while (--Width)
                        *Out++ = ' ';
                *Out++ = (char)va_arg (Args, int);

                Ptr++;
                continue;
            case 's':
                {
                    char *Src = (char *)va_arg (Args, char *);
                    if (Src == NULL)
                        Src = "(null)"; // 如果是 NULL 就填 "(null), 总不可能去访问 0x00 吧 QaQ"
                    for (char *p = Src;p && *p;p++)
                        Width--;
                    while (Width-- > 0)
                        *Out++ = ' ';
                    while (*Src)
                        *Out++ = *Src++;

                    Ptr++;
                    continue;
                }
            case 'p':
                Radix = 16;
                Length = 2;
                Flgs |= SPECIAL;
                break;

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '0':
                Offset = __Int (Ptr, &Width);
                Ptr += Offset - 1;
                goto ParseArgs;
            case '*':
                Width = va_arg (Args ,int);
                if (Width < 0)
                    Flgs |= LEFT; // 左对齐
                goto ParseArgs;
        }

        u64 Value;
        bool Minus = false;

        if (Length == 0)
        {
            Value = va_arg (Args, unsigned int);
            if (Signed && (int)Value < 0) {
                Minus = true;
                Value = -(int)Value; // 符号位将在最后于字符串上添上.
            }
        }
        else if (Length == 1)
        {
            Value = va_arg (Args, unsigned long);
            if (Signed && (long)Value < 0) {
                Minus = true;
                Value = -(long)Value;
            }
        }
        else if (Length == 2)
        {
            Value = va_arg (Args, unsigned long long);
            if (Signed && (long long)Value < 0) {
                Minus = true;
                Value = -(long long)Value;
            }
        }

        /* 每一次添加字符('+','-',' '...)都会导致 Siz 减小,
           这么做在于最后可以直接使用 Siz 来进行填充操作. */
        int64 Siz = Width;
        Siz -= __Number (Tmp, Value, Radix, UpperCase);

        char Prefix = 0;
        if (Signed && Radix == 10)
        {
            if (Flgs & SIGN) {
                Siz--;
                if (Minus)
                    Prefix = '-';
                else
                    Prefix = '+';
            } else if (Flgs & SPACE) {
                Siz--;
                Prefix = ' ';
            }
        }

        if (Flgs & SPECIAL) {
            Siz -= (Radix == 16) ? 2 :
                   (Radix == 8 ) ? 1 : 0;
        }

        if (Flgs & ZERO)
        {
            if (Prefix) *Out++ = Prefix;
            /* "0x" "0X" "0" for SPECIAL */
            if (Flgs & SPECIAL && Radix != 10)
            {
                *Out++ = '0';
                if (Radix == 16) {
                    *Out++ = UpperCase ? 'X' : 'x';
                }
            }
        }

        /* Padding */
        if (!(Flgs & LEFT) && Siz > 0)
        {
            char Pad = Flgs & ZERO ? '0' : ' ';

            while (Siz--) {
                *Out++ = Pad;
            }
        }

        /* Symbol and others after padding if that is not filled with '0' -> "    0x91d" */
        if (!(Flgs & ZERO))
        {
            if (Prefix) *Out++ = Prefix;
            /* "0x" "0X" "0" for SPECIAL */
            if (Flgs & SPECIAL && Radix != 10)
            {
                *Out++ = '0';
                if (Radix == 16) {
                    *Out++ = UpperCase ? 'X' : 'x';
                }
            }
        }

        for (char *p = Tmp;*p;) {
            *Out++ = *p++;
        }

        Ptr++;
    }

    return (int64)(Out - Buffer);
}

#define PRINTK_BUFFER_MAX 256

size_t PrintK (char *Format, ...)
{
    va_list Args;
    va_start (Args, Format);

    char Buffer[PRINTK_BUFFER_MAX] = {0};

    size_t i = VSPrint (Buffer, Format, Args);

    printf ("%s",Buffer);

    va_end (Args);

    return i;
}

int main ()
{
    char test;
    PrintK ("%233p\n",&test);
    return 0;
}

# `gST->ConOut->OutputString`
是Uefi下最基础的打印函数,通过Uefi提供的接口进行打印字符串,不支持"格式化".

## 原型

```cpp

/* File : MdePkg/Include/Protocol/SimpleTextOut.h */
/* gST->ConOut->OutputString(This,String) */

/**
  Write a string to the output device.

  @param  This   The protocol instance pointer.
  @param  String The NULL-terminated string to be displayed on the output
                 device(s). All output devices must also support the Unicode
                 drawing character codes defined in this file.

  @retval EFI_SUCCESS             The string was output to the device.
  @retval EFI_DEVICE_ERROR        The device reported an error while attempting to output
                                  the text.
  @retval EFI_UNSUPPORTED         The output device's mode is not currently in a
                                  defined text mode.
  @retval EFI_WARN_UNKNOWN_GLYPH  This warning code indicates that some of the
                                  characters in the string could not be
                                  rendered and were skipped.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_TEXT_STRING)(
  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
  IN CHAR16                                 *String
  );

```

## 用法

由于我们是使用`gST->ConOut->OutputString`进行调用所以要包含头文件:`#include <Library/UefiBootServicesTableLib.h>`

---

传入参数
1. `EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This`,这里即传入这个函数指针所在的结构体`gST->ConOut`
2. `CHAR16 *String`,需要打印的字符串,`CHAR16`类型

---

## Example

```cpp

gST->ConOut->OutputString(gST->ConOut,L"Hello world!\n");

```

# `Print`
可支持格式化操作,基于`gST->ConOut->OutputString`

## 原型

```cpp
/* File : Library/UefiLib.h */

/** 
  Prints a formatted Unicode string to the console output device specified by 
  ConOut defined in the EFI_SYSTEM_TABLE.

  This function prints a formatted Unicode string to the console output device 
  specified by ConOut in EFI_SYSTEM_TABLE and returns the number of Unicode 
  characters that printed to ConOut.  If the length of the formatted Unicode 
  string is greater than PcdUefiLibMaxPrintBufferSize, then only the first 
  PcdUefiLibMaxPrintBufferSize characters are sent to ConOut.
  If Format is NULL, then ASSERT().
  If Format is not aligned on a 16-bit boundary, then ASSERT().
  If gST->ConOut is NULL, then ASSERT().

  @param Format   A null-terminated Unicode format string.
  @param ...      The variable argument list whose contents are accessed based 
                  on the format string specified by Format.
  
  @return Number of Unicode characters printed to ConOut.

**/
UINTN
EFIAPI
Print (
  IN CONST CHAR16  *Format,
  ...
  );

```

## 用法
类似于C标准库下的`printf`,但传入的字符串必须为`CHAR16`型

## Example

```cpp

Print(L"Hello World\n");

```

---

格式化输出:
```cpp

INT32 Num = 255;
Print(L"The Num is %d\n",Num);
Print(L"The Num is %x\n",Num);

```

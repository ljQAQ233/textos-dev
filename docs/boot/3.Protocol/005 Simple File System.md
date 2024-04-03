# Overview

以下接口由 **BootService** 提供!!!

## OpenProtocol

```
typedef
EFI_STATUS
(EFIAPI *EFI_OPEN_PROTOCOL)(
  IN  EFI_HANDLE                Handle,
  IN  EFI_GUID                  *Protocol,
  OUT VOID                      **Interface, OPTIONAL
  IN  EFI_HANDLE                AgentHandle,
  IN  EFI_HANDLE                ControllerHandle,
  IN  UINT32                    Attributes
  );
```

- `Handle` -> 已安装此 Protocol 的句柄
- `Protocol` -> 要打开的 Protocol 的 GUID
- `Interface` -> 返回的 Protocol 实例,无则`NULL`,可选项
- `AgentHandle` -> 程序Handle

- `Attributes` -> 打开 Protocol 的 参数 / 模式

```c++
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001 // 类似HandleProtocol()
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002 // 从句柄上获取
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004 // 测试是否存在
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020 // 获取Protocol独占权
```

## LocateHandle

```c++
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE)(
  IN     EFI_LOCATE_SEARCH_TYPE   SearchType,
  IN     EFI_GUID                 *Protocol,    OPTIONAL
  IN     VOID                     *SearchKey,   OPTIONAL
  IN OUT UINTN                    *BufferSize,
  OUT    EFI_HANDLE               *Buffer
  );
```

```c++
typedef enum {
  /// 所有 Handle
  AllHandles,
  ///
  /// Retrieve the next handle fron a RegisterProtocolNotify() event.
  ///
  ByRegisterNotify,
  /// 找出所有 Protocol (GUID)对应的Handle,忽略 SearchKey
  ByProtocol
} EFI_LOCATE_SEARCH_TYPE;
```

在操作成功后,`Buffer` 中Handle的个数为`BufferSize / sizeof(EFI_HANDLE)`.

**请在Protocol打开后完成FreePool操作!!!**

## LocateHandleBuffer

它与`LocateHandle`的最大区别在于,`LocateHandle`需要一个预先准备好的Buffer,而它会自己创建.

```c++
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_HANDLE_BUFFER)(
  IN     EFI_LOCATE_SEARCH_TYPE       SearchType,
  IN     EFI_GUID                     *Protocol,      OPTIONAL
  IN     VOID                         *SearchKey,     OPTIONAL
  IN OUT UINTN                        *NoHandles,
  OUT    EFI_HANDLE                   **Buffer
  );
```

# GUID 的定义

一般在头文件中写的清清楚楚,但是是宏的形式,并未有实际定义全局变量.我们可以采取以下解决方案:

1. 不使用全局变量,自己定义

```c++
EFI_GUID EfiSimpleFileSystemProtocolGuid = (EFI_GUID)EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
```

2. 使用全局变量,在inf文件中声明,在编译生成AutoGen.c时定义

```cfg
[Protocols]
    gEfiSimpleFileSystemProtocolGuid
```

# 参考

- `src/boot/SigmaBootPkg/File.c`


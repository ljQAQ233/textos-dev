# Overview

Protocol 都应该含有以下定义:

- GUID - 全局唯一标识符
- Protocol Interface Structure - Protocol接口结构体
- Protocol Services - Protocol服务(接口函数)

## GUID

> Globally Unique IDentifier

每一个 Protocol 有着唯一的 GUID,它将在操作 Protocol 时使用.

```c++
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
  { \
    0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } \
  }
```

## Protocol Interface Structure

```c++
///
/// Provides a basic abstraction to set video modes and copy pixels to and from 
/// the graphics controller's frame buffer. The linear address of the hardware 
/// frame buffer is also exposed so software can write directly to the video hardware.
///
struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
  EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE  QueryMode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE    SetMode;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT         Blt;
  ///
  /// Pointer to EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE data.
  ///
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE        *Mode;
};
```

## Protocol Services

调用上述 :

```c++
typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
  IN  UINT32                       ModeNumber
  );
```

This 指针区别于 C++,这里是需要手动传参,且这个指针是 **Protocol Interface Structure**

# Open

## LocateProtocol

对于一般只有一个的设备,我们可以省去手动选择 Handle 的过程.

如你所见啊，真的有简单的方法!

```c++
typedef
EFI_STATUS
(EFIAPI *EFI_LOCATE_PROTOCOL)(
  IN  EFI_GUID  *Protocol,
  IN  VOID      *Registration, OPTIONAL // 可选项
  OUT VOID      **Interface
  );
```

它将返回第一个找到的 **Protocol实例**

这里以 `GraphicsOutpurProtocol` 为例

```c++
EFI_STATUS InitializeGraphicsServices ();
```

- `src/boot/SigmaBootPkg/Graphics.c`

### 缺点

当有多个Protocol实例时不能选择.

## OpenProtocol

打开 Protocol 的一般步骤:
1. LocateHandle
2. OpenProtocol

Details ->
1. LocateHandle
    1. 取锁
    2. ByProtocol ->
    3. 查找 ProtEntry
    4. 寻找 `ProtEntry->Portocols` 下的所有 `PROTOCOL_INTERFACE`, `PROTOCOL_INTERFACE.Handle` 即使 Handle

Ext -> Howt to get the num of handles which support this prot?
 Ans -> Pass 0 as the param `BufferSiz` so that the `BufferSiz` will be set by our uefi-core.

2. OpenProtocol

# Explore a lot...

`docs/boot/4.Core/debug.md`

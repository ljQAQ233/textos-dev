# Start here ->

将 Makefile 中的无关调试的 OVMF 镜像去掉, 只编译 NOOPT.

---

Aims ->
 - Handle database
 - Protocol database

# EFI_DRIVER_BINDING_PROTOCOL

```cpp
/**

  Tests to see if this driver supports a given controller. If a child device is provided, 

  it further tests to see if this driver supports creating a handle for the specified child device.



  This function checks to see if the driver specified by This supports the device specified by 

  ControllerHandle. Drivers will typically use the device path attached to 

  ControllerHandle and/or the services from the bus I/O abstraction attached to 

  ControllerHandle to determine if the driver supports ControllerHandle. This function 

  may be called many times during platform initialization. In order to reduce boot times, the tests 

  performed by this function must be very small, and take as little time as possible to execute. This 

  function must not change the state of any hardware devices, and this function must be aware that the 

  device specified by ControllerHandle may already be managed by the same driver or a 

  different driver. This function must match its calls to AllocatePages() with FreePages(), 

  AllocatePool() with FreePool(), and OpenProtocol() with CloseProtocol().  

  Because ControllerHandle may have been previously started by the same driver, if a protocol is 

  already in the opened state, then it must not be closed with CloseProtocol(). This is required 

  to guarantee the state of ControllerHandle is not modified by this function.



  @param[in]  This                 A pointer to the EFI_DRIVER_BINDING_PROTOCOL instance.

  @param[in]  ControllerHandle     The handle of the controller to test. This handle 

                                   must support a protocol interface that supplies 

                                   an I/O abstraction to the driver.

  @param[in]  RemainingDevicePath  A pointer to the remaining portion of a device path.  This 

                                   parameter is ignored by device drivers, and is optional for bus 

                                   drivers. For bus drivers, if this parameter is not NULL, then 

                                   the bus driver must determine if the bus controller specified 

                                   by ControllerHandle and the child controller specified 

                                   by RemainingDevicePath are both supported by this 

                                   bus driver.



  @retval EFI_SUCCESS              The device specified by ControllerHandle and

                                   RemainingDevicePath is supported by the driver specified by This.

  @retval EFI_ALREADY_STARTED      The device specified by ControllerHandle and

                                   RemainingDevicePath is already being managed by the driver

                                   specified by This.

  @retval EFI_ACCESS_DENIED        The device specified by ControllerHandle and

                                   RemainingDevicePath is already being managed by a different

                                   driver or an application that requires exclusive access.

                                   Currently not implemented.

  @retval EFI_UNSUPPORTED          The device specified by ControllerHandle and

                                   RemainingDevicePath is not supported by the driver specified by This.

*/

typedef
EFI_STATUS
(EFIAPI *EFI_DRIVER_BINDING_SUPPORTED)(
  IN EFI_DRIVER_BINDING_PROTOCOL            *This,
  IN EFI_HANDLE                             ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL               *RemainingDevicePath OPTIONAL
  );

```


# 初始化过程

## InitializeQemuVideo

serial output:
```
    PDB = /home/maouai233/TextOS/Build/Boot/OVMF_X64/NOOPT_GCC5/X64/OvmfPkg/QemuVideoDxe/QemuVideoDxe/DEBUG/QemuVideoDxe.dll
Loading driver at 0x0000274F000 EntryPoint=0x00002757156 QemuVideoDxe.efi
InstallProtocolInterface: BC62157E-3E33-4FEC-9920-2D3B36D750DF 3096B98
ProtectUefiImageCommon - 0x3096D40
  - 0x000000000274F000 - 0x000000000000C3C0
```

- Call Stack
  - `InitializeQemuVideo (ImageHandle, SystemTable)`
  - `ProcessModuleEntryPointList (...)`
  - `_ModuleEntryPoint (...)`

```c++
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gQemuVideoDriverBinding,
             ImageHandle,
             &gQemuVideoComponentName,
             &gQemuVideoComponentName2
             );
```

NOTE: !!! 此处的 Interface 并非 PROTOCOL_INTERFACE, 而是 PROTOCOL_INTERFACE 中的一个成员, 为 Protocol 本体, 相当于 此 C++ 的一个实例, 所属的类里的 public 方法以及变量都在这里面 !!!

1. `EfiLibInstallDriverBindingComponentName2`
    1. 更新 DriverBinding (EFI_DRIVER_BINDING_PROTOCOL) -> `gQemuVideoDriverBinding` 的 ImageHandle 以及 DriverBindingHandle (DriverBinding 将要被安装至的句柄)
    2. 在 `DriverBinding->DriverBindingHandle` 上安装 Protocol
        1. HandleProtocol 检测 Handle 是否已经存在该 Protocol
        2. 获得 Protocol 数据库的锁
        3. 在 Protocol 数据库中使用 Guid 查找 ProtocolEntry
            1. 如果没有找到且参数中的 `Create` 为真则创建一个新项
                1. 初始化成员
                2. 插入 Protocol 数据库
        4. UserHandle 已存在, 不需要创建
        5. ~~断言以维持 PROTOCOL_INTERFACE 的唯一性~~
        6. 初始化 PROTOCOL_INTERFACE
            1. 挂载至 Handle
            2. 绑定 ProtocolEntry
            3. Interface 成员 -> 初始化 `OpenList` & `OpenListCount`
            4. 将 `Link` 插入 `Handle->Protocols`, 以后可以通过此 Handle 查询该 Handle 支持的 Protocol
            5. 将 `ByProtocol` 插入 `ProtEntry->Protocols` , 以后可以通过此 ProtEntry 查询 该类 Protocol 的所有 "实例".
        7. 如果是 **Notify**, 还要为该 ProtEntry 下挂的所有 Protocol 开启 Notification
        8. 释锁

NOTE: 在 `CoreInstallProtocolInterfaceNotify` 中
```c++
  if (*UserHandle != NULL) {
    Status = CoreHandleProtocol (*UserHandle, Protocol, (VOID **)&ExistingInterface);
    if (!EFI_ERROR (Status)) {
      return EFI_INVALID_PARAMETER;
    }
  }
```
说明不能在相同的 Handle 上安装同类的 Protocol

TODO: Notification -> What's the fk?

## QemuVideoControllerDriverSupported

1. Open the PCI I/O Protocol
2. Read the PCI Configuration Header from the PCI Device
3. 检测图像卡类型
4. Close PCI I/O Protocol
5. 返回成功, 准备启动 `QemuVideoControllerDriverStart`

NOTE: 此接口会被 DXE Dispatcher 调用数次!!!

```
Found PCI display device
QemuVideo: QEMU Standard VGA detected
```

## QemuVideoControllerDriverStart



# OpenProtocol

1. 检测 `UserHandlle` 是否有效
  - 从 `gHandleList` 出发, 检测 `UserHandle` 是否在此链表之内
2. 检测 `Attributes` 即 "The open mode of the protocol interface specified by Handle and Protocol", 此处是 `EFI_OPEN_PROTOCOL_BY_DRIVER`.
  - 检测 `ImageHandle` 是否在数据库内.
  - 检测 `ControllerHandle` 是否在数据库内.
3. 获得锁
4. `CoreGetProtocolInterface()` 通过 `EFI_GUID` 获取 Interface
  - 遍历 (IHANDLE)UserHandle 上的 `PROTOCOL_INTERFACE` 即 Protocols, 比较 GUID

# EFI_PCI_IO_PROTOCOL


# gBS->InstallMultipleProtocolInterfaces

第一个参数为 Protocol 将被安装到的 Handle, 接下来的参数都是 可变长参, 一个 Guid 一个 Protocol, 最终以 NULL 结尾

# Achieve our aims!

```c++
///
/// PROTOCOL_ENTRY - each different protocol has 1 entry in the protocol
/// database.  Each handler that supports this protocol is listed, along
/// with a list of registered notifies.
///
typedef struct {
  UINTN               Signature;
  /// Link Entry inserted to mProtocolDatabase
  LIST_ENTRY          AllEntries;  
  /// ID of the protocol
  EFI_GUID            ProtocolID;  
  /// All protocol interfaces
  LIST_ENTRY          Protocols;     
  /// Registerd notification handlers
  LIST_ENTRY          Notify;                 
} PROTOCOL_ENTRY;
```

`Item = CR(Link, PROTOCOL_ENTRY, AllEntries, PROTOCOL_ENTRY_SIGNATURE);` => AllEntries 是将 Protocol 数据库连接起来的重要节点

易知: `PROTOCOL_ENTRY` 由多个 同种的 Protocol 共享.

---

```c++
///
/// IHANDLE - contains a list of protocol handles
///
typedef struct {
  UINTN               Signature;
  /// All handles list of IHANDLE
  LIST_ENTRY          AllHandles;
  /// List of PROTOCOL_INTERFACE's for this handle
  LIST_ENTRY          Protocols;      
  UINTN               LocateRequest;
  /// The Handle Database Key value when this handle was last created or modified
  UINT64              Key;
} IHANDLE;
```

AllHandles 是将 Handle 数据库联系起来的重要节点.
Key 存储的值是 当前的 gHandleDatabaseKey, 即 **此时添加或修改数据库的总次数**, 由于数据库上锁, 此应保持唯一性.

---

```c++
///
/// PROTOCOL_INTERFACE - each protocol installed on a handle is tracked
/// with a protocol interface structure
///
typedef struct {
  UINTN                       Signature;
  /// Link on IHANDLE.Protocols
  LIST_ENTRY                  Link;   
  /// Back pointer
  IHANDLE                     *Handle;  
  /// Link on PROTOCOL_ENTRY.Protocols
  LIST_ENTRY                  ByProtocol; 
  /// The protocol ID
  PROTOCOL_ENTRY              *Protocol;  
  /// The interface value
  VOID                        *Interface; 
  /// OPEN_PROTOCOL_DATA list
  LIST_ENTRY                  OpenList;       
  UINTN                       OpenListCount;
} PROTOCOL_INTERFACE;
```

`ByProtocol` 与 `PROTOCOL_ENTRY` 的 `Protocols` 相连

`@CoreGetNextLocateByProtocol` ->

```c++
    //
    // Get the handle
    //
    Prot = CR(Link, PROTOCOL_INTERFACE, ByProtocol, PROTOCOL_INTERFACE_SIGNATURE);
```

# About `_ModuleEntryPoint` & Image's src

EFI_STATUS CoreStartImage (ImageHandle, ExitDataSize, ExitData OPT);

- 启动 -> 调用入口函数
- CoreExit
  - `LongJump (Image->JumpContext, (UINTN)-1);` -> return to StartImage

驱动程序也是一个模块, 当然有自己的 `_ModuleEntryPoint`, 这个模块被加载, 处于 DXE 阶段, 由 **DXE Dispatcher** (DXE 派遣器) 发起.

DXE Dispatcher 有一个 `mScheduledQueue`, 以 DriverEntry 为其节点, 当它不为空队时停止派遣, 即 `mScheduledQueue` 存储的 DriverImage 被处理完后停止.

```c++
EFI_STATUS
EFIAPI
CoreLoadImage (
  IN BOOLEAN                    BootPolicy,
  IN EFI_HANDLE                 ParentImageHandle,
  IN EFI_DEVICE_PATH_PROTOCOL   *FilePath,
  IN VOID                       *SourceBuffer   OPTIONAL,
  IN UINTN                      SourceSize,
  OUT EFI_HANDLE                *ImageHandle
  );
```

`ImageHandle` 在 `CoreLoadPeImage` 加载好 PE/COFF 映像后已经取得, 若成, 则位于 `LOADED_IMAGE_PRIVATE_DATA.Handle`.

加载成功映像后, 返回的 `ImageHandle` 将在 `CoreStartImage` 时 传入.

# Step out!

在 SigmaBoot 给出的示例中:
```c++
Status = gBS->LocateHandleBuffer (
        ByProtocol,
        &gEfiSimpleFileSystemProtocolGuid,
        NULL,
        &HandleCount,&HandleBuffer
    );
```

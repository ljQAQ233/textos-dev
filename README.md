# TextOS

这是一个 Uefi 引导的 x64 操作系统项目.

- Boot : SigmaBoot
- Opearting System : TextOS

- [Gitee | canyan233](https://gitee.com/canyan233)
- [GitHub | ljQAQ233](https://github.com/ljQAQ233)
- [Bilibili | maouai233](https://space.bilibili.com/503518259)

在B站不定期更新,有错误什么的请提交issue/PR,谢谢!学生党,更新慢,请见谅.

# Feature

- [x] uefi
    - [x] debug support
    - [x] bmp display
    - [x] configuration
    - [x] memory map
    - [x] resolution set
    - [x] elf loader
    - [x] boot arguments

---

- 内存管理
  - [x] 物理内存管理
  - [ ] 虚拟内存管理
  - [x] 内核堆内存管理
  - [x] 内核重映射

- 任务管理
  - [ ] 信号

- 可执行程序
  - [x] ELF 加载
  - [ ] ELF 动态链接

- 系统调用
  - `textos/syscall.h`

- 设备驱动
  - [x] 串口输出 (COM1)
  - [x] QEMU debugcon
  - [x] 控制台
    - [x] CSI
  - [x] RTC 时钟
  - [x] PS/2 键盘
  - [x] IDE 硬盘
  - [x] E1000 网卡
  - [x] RTL8139 网卡
  - [ ] FPU 协处理器
  - 虚拟设备
    - [ ] mem
    - [x] zero
    - [x] null 
    - [ ] full
    - [ ] port
  - [x] acpi / lai
  - pci
    - [x] INTX 传统中断
    - [x] MSI 中断 (test needed)

- 文件系统
  - [x] pipe
  - [x] fat32
  - [ ] minix
  - [ ] iso9660

- 网络接口
  - [x] ethernet
  - [x] arp
  - [x] ipv4
  - [x] icmp
  - [x] udp (incompleted)
  - [ ] tcp (incompleted)
  - [x] dns (标准库提供 / `app/dns.c`)
  - socket
    - `textos/net/socket.h`

---

- app 大多可以直接在 linux 上运行 ~~因为我直接把 linux 的 syscall number 搬过来了~~
- app 使用自制的 C library

## 系统调用

# 目录结构

> 与一般的目录结构大不相同...

现在就很相同了 [why](docs/history.md)

- build 构建输出
- docs 文档
- src
   - base           开发资源
   - boot    
     - Edk2         基于EDKII项目改,使用makefile
     - SigmaBootPkg Boot源码
   - config         Makefile配置
   - include        头文件目录
      - boot        SigmaBootPkg的头文件
   - utils          工具
   - kernel         内核源代码
   - resource       资源文件

---

使用 Makefile 硬构强力驱动 /doge [make](#make)

# make

- `make -C src qemu` -> running on Qemu
- `make -C src qemug` -> start kernel debugging
- `make -C src qemubg` -> start uefi debugging
- `make -C src compile_commands.json` -> AutoGen CompileCommands

- `make -C src ovmf`
- `make -C src ovmf-debug`
- `make -C src ovmf-noopt`

---

Qemu启动可选项:

- `QEMU_GPY`
   - false -> 不显示Qemu图形化窗口

# 学习 / 开发此项目

## 初始化

## compile_commands.json

```shell
make -C src compile_commands.json
```

将 `src/utils/compile_commands.json`添加到配置中.

如果你使用的是 VSCode , 将下述片段根据你的实际情况添加到 `.vscode/c_cpp_properties.json` 中

```
"compilerPath": "/usr/bin/gcc",
"cStandard": "c11",
"cppStandard": "c++14",
"intelliSenseMode": "linux-gcc-x64",
```

VSCode可以配置`includePath`

# 参考软件版本

- Provide
   - EDKII - vUDK2018
- Private
   - GCC - `gcc 14.2.1`
   - GDB - `gdb 15.1` (with `expat`)
   - QEMU - `QEMU emulator version 9.0.2`

# 参考资料

## 文档

- docs

- [Intel® 64 and IA-32 Architectures Software Developer’s Manual](https://www.intel.cn/content/www/cn/zh/developer/articles/technical/intel-sdm.html)
- [Unified Extensible Firmware Interface (UEFI) Specification Version 2.9 March 2021](https://uefi.org/specifications)

**注: Uefi文档与当前版本有出入**

## 网站

- [osdev](https://wiki.osdev.org)

## 项目

- <https://github.com/stevenbaby/onix>
- <https://github.com/Minep/lunaix-os>

- <https://gitee.com/tanyugang/UEFI>
- <https://github.dev/aar10n/osdev>
- <https://gitee.com/luobing4365/uefi-practical-programming>

## 视频

- [ 踌躇月光 | 操作系统实现 ](https://space.bilibili.com/491131440/channel/collectiondetail?sid=146887)
- [ Lunaixsky | 从零开始自制操作系统 ](https://space.bilibili.com/12995787/channel/collectiondetail?sid=196337)

# 其他

## git commit format

```
[doc|fix|update|bili] (details)
```

# LICENSE

[MIT License] (LICENSE)

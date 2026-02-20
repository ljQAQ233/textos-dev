# TextOS

这是一个 Uefi 引导的 x64 操作系统项目, 同时支持 multiboot.

- Boot : SigmaBoot
- Opearting System : TextOS

---

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
  - [ ] cmd line args
- [x] multiboot
- [ ] multiboot2

---

- 内存管理
  - [x] 物理内存管理
  - [ ] 虚拟内存管理
  - [x] 内核堆内存管理
  - [x] 内核重映射

- 任务管理
  - [x] 信号

- 可执行程序
  - [x] ELF 加载
  - [x] ELF 动态链接

- 系统调用
  - [`textos/syscall.h`](src/include/textos/syscall.h)

- 设备驱动
  - [ ] event
    - [x] keyboard
    - [ ] mouse
    - [ ] 文件抽象
  - [x] 串口输出 (COM1 ~ COM4)
  - [x] QEMU debugcon
  - [x] 控制台
    - [x] CSI
  - [ ] tty
    - [x] simple tty driver
    - [x] tty1
    - [x] ttySx
  - [x] RTC 时钟
  - [x] PS/2 键盘
  - [x] IDE 硬盘
  - [ ] AHCI / SATA
  - [x] E1000 网卡
  - [x] RTL8139 网卡
  - [x] FPU 协处理器
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
    - [x] minix1
    - [ ] minix2
    - [ ] minix3
  - [ ] iso9660
  - [x] procfs
  - [x] tmpfs
  - [ ] qemu_fw_cfg

- 网络接口
  - [x] ethernet
  - [x] arp
  - [x] ipv4
  - [x] icmp
  - [x] udp (incompleted)
  - [x] tcp (incompleted)
  - [x] dns (标准库提供 / `app/dns.c`)
  - [ ] unix socket
  - socket
    - `textos/net/socket.h`

- 图形化
  - [ ] lvgl
    - [x] framebuffer
    - [ ] mouse
    - [ ] keyboard
  - [ ] x11
  
- 多任务
  - [x] percpu (mycpu)
  - [ ] clone
  - [ ] tls

---

- app 大多可以直接在 linux 上运行 ~~因为我直接把 linux 的 syscall number 搬过来了~~ (二进制兼容)
- app 使用自制的 C library

## 系统调用

# 目录结构

> 与一般的目录结构大不相同...

现在就很相同了 [why](docs/history.md)

- build 构建输出
- docs 文档
- src
   - app            用户程序
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

- 测试
  - `make -C src qemu` -> running on Qemu
  - `make -C src qemug` -> start kernel debugging
  - `make -C src qemubg` -> start uefi debugging
  - `make -C src compile_commands.json` -> AutoGen CompileCommands

- 固件
  - `make -C src ovmf`
  - `make -C src ovmf-debug`
  - `make -C src ovmf-noopt`

---

- `QEMU_HOME`
- `QEMU_GPY` - false -> 不显示Qemu图形化窗口
- `QEMU_MEM` - 内存大小默认 64M
- `QEMU_LOG` - QEMU 日志输出到哪?
- `BOOT_MODE`
  - `emulti` - `-kernel` 加载 内核 直接启动
  - `efi` - UEFI / OVMF 环境启动

# 学习 / 开发此项目

## 初始化

## git

使用本仓库:

```shell
git clone https://github.com/ljqaq233/textos-pre
```

edk2 子模块也必须要克隆:

```shell
git submodule update --init --progress
```

edk2 编译依赖: 见 [OSDev](https://wiki.osdev.org/EDK2)

## compile_commands.json

本项目使用 bear + make 进行 `compile_commands.json` 的构建

```
# archlinux
sudo pacman -S bear
```

```shell
make -C src compile_commands.json
```

将 `compile_commands.json`添加到配置中. VSCode 可以配置`includePath`

## NixOS

```shell
nix-shell shell.nix
```

即可在 shell 环境中编译项目

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

> 原 [预发布仓库](https://github.com/ljQAQ233/textos-pre) 旨在提前原来的 bilibili 同步仓库 (也就是这一个仓库) 进行提交, 已经被 archive, 因为只有这一个仓库会受到大家的点击 (大概是我将连接放在前面的缘故...) -- 2025/08/03

- OS
  - <https://github.com/Jimx-/lyos/>
  - <https://github.com/klange/toaruos>
  - <https://github.com/Minep/lunaix-os>
  - <https://github.com/stevenbaby/onix>

- UEFI
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

[MIT License](./LICENSE)

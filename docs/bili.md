# bilibili

Bilibili 视频暂时是无法更新了, 在此列出可能的规划.

- [000](./tidyup.md) 工作区整理
- [001](./origin) 序章
- [002](./boot/debug/visual.md.md) UEFI 可视化调试
  - [.1]() udk 加载脚本分析
- [003](./boot/debug/macros.md) 基础调试宏
- [004](./boot/protocol/protocol.md) Procotol
  - [.1](./boot/protocol/protocol.md) Protocol 组织
- [005](./boot/protocol/sfs) Simple File System Protocol
- [006](./boot/file/open-rw-wind.md) 文件打开及读写、定位
- [007](./boot/file/info-of-file.md) 文件信息
  - .1 文件完全读取
- [008](./boot/file/close-rm-override.md) 文件关闭、删除及覆盖
- [009](./boot/graphics/resolution.md) 显示器分辨率设置
- [010](./boot/graphics/operations.md) 图形操作
- [011](./boot/graphics/bmp-show.md) Bmp位图显示
  - [.1](./boot/graphics/cls.md) 清屏操作
- [012](./boot/graphics/logo-show.md) Logo 显示
- [013](./boot/config/config.md) 启动配置文件
- [014](./boot/runtime/barebone-asmkern.md) 一个汇编内核的裸机加载运行
- [015](./boot/runtime/elf-and-kernel.md) Elf文件及内核加载
- [016](./boot/runtime/bootsrv-mata.md) 退出启动时服务
- [017](./kernel/organization.md) 内核组织
- [018](./kernel/pagetable.md) 内核页表
- [019](./kernel/debug.md) 内核调试
- [020](./kernel/params.md) 内核参数传递
- [021](./kernel/graphics/basic.md) 基础图像操作
- [022](./kernel/graphics/font.md) 字体显示
  - [022](./kernel/graphics/font.md) 字体显示
- [023](./kernel/device/console.md) 控制台
- [024](./lang/vaargs.md) 可变参数函数
- 025 printk
- 026 内核调试宏
- 027 断言 ASSERTK
  [.1](./lang/static-assert.md)
- 028 字符串处理
- [029](./kernel/memory/glob-desc-tab.md) 内核全局描述符
- [030](./kernel/interrupt/intr-x86.md) 中断
- 031 输入输出
- [032](./kernel/device/serial.md) 串口
- 033 panic
- 034 物理内存管理
- [035](./kernel/memory/virt-mem-map.md) 虚拟内存映射
- [036](./kernel/memory/kernel-heap.md) 堆
- [037](./kernel/memory/kernel-remap.md) 内核重映射
- [038](./kernel/interrupt/apic-acpi.md) acpi 与 apic
- 039 键盘中断
  - 这里开始引入 GSI map
- [040](./kernel/interrupt/apic-timer.md) APIC Timer
- [041](./kernel/task/task.md) 任务
- 042 任务睡眠
- 043 任务阻塞
- 044 设备管理
- 045 键盘扫描码
- 046 ???
- [047](./kernel/filesystem/vfs-and-fat32.md) 虚拟文件系统
  - [.1](./kernel/filesystem/analysis-01.md) FAT32 分析 1
  - [.2](./kernel/filesystem/analysis-02.md) FAT32 分析 2
- [048](./kernel/device/clock.md) 墙上时钟
- [049](./kernel/userspace/userspace.md ) 用户态
- [050](./kernel/userspace/syscall.md) 系统调用
- [051](./kernel/userspace/syscall.md) fork (deep-fork)
- 052 文件操作

# 游离态文档

- [xxx](./kernel/network/e1000.md) e1000
- [xxx](./kernel/network/arp.md) arp
- [xxx](./kernel/network/tcp.md) tcp
- [xxx](./kernel/userspace/user-manager.md)  用户管理
- [xxx](./kernel/userspace/user-ids-rights.md)  权限 与 ID


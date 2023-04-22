# Overview

## 前端

- VSCode
- VimSpector

## 后端

- gdb
- udkdebugger

# 准备

## 中断gdb

为什么? 编译得到的 `.debug` 文件中的符号需要通过其具体的加载地址加上一个偏移量所得.中断 `UefiMain`,是因为它是入口.
在中断之后,gdb停下,脚本自动加载,符号自然就上去了.

---

中断 gdb 运行以**加载调试符号**

### `int $3` -> Used

在`UefiMain`的适当位置下个"断点"

这里我用自己写的，要不然可会阻断**RELEASE**的执行.

单步调试中断,也可以断gdb

```c++
#ifdef __SRC_LEVEL_DEBUG
  #include <Library/UefiLib.h>
  #define Breakpoint() __asm__ volatile ("int $1")
#else
  #define Breakpoint()
#endif
```

我们需要调试时,把`__SRC_LEVEL_DEBUG`定义一下即可!

### CpuBreakpoint

- `Src/Include/Boot/Boot.h`

```c++
#ifdef __SRC_LEVEL_DEBUG
  #include <Library/UefiLib.h>
  #define Breakpoint() CpuBreakpoint()
#else
  #define Breakpoint()
#endif
```

我们需要Cpu断点时,把`__SRC_LEVEL_DEBUG`定义一下即可!

# 启动

```shell
make -C Src qemubg
```

关闭图形化输出:

```shell
make -C Src QEMU_GPY=false qemubg
```

在调试器加载时加载以下脚本：

* `Src/Utils/Udk/script/udk_gdb_script`

> Press `F5` to start!
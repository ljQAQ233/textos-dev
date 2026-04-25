# mycpu

```c++
#define MYCPU_DEFINE(type, name)
#define MYCPU_DECLARE(type, name)

#define get_cpu_base(cpu)
#define get_cpu_var(cpu, name)
#define get_cpu_varptr(cpu, name)

#define mycpu_var(name)
#define mycpu_varptr(name)
```

# impl

## 参考

- linux
- [lyos](https://github.com/Jimx-/lyos/)

## 思路

`MYCPU_DEFINE` 创建的变量存储在一个特殊的 节: `.data.mycpu`. 这个节存储的是 变量的 初始值, 在运行时将它复制到具体的 内存区域.

`MYCPU_DECLARE` 是为外部访问提供信息 (符号名 / 偏移量).

在 `x86_64` 上, 可以将内核的 `gs_base` 设置为 **此 cpu** 的 mycpu **基地址**(`void *mycpu`), 即可快速访问本 cpu 的变量. 在 lyos 上, **将基地址设置为 `mycpu - __mycpu_start`**, 这样, 使用 `gs:&name` 即可直接访问

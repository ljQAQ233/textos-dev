# 内核组织

# Overview

```
src
├── config
│   └── kernel.mk
├── include
│   └── textos
│       ├── base.h
│       ├── textos.h
│       └── type.h
├── kernel
│   ├── arch
│   │   └── x64
│   ├── init.c
│   ├── linker.ld
│   └── Makefile
└── Makefile
```

- `config/kernel.mk` -> 编译参数等设置
- `include/textos` -> 内核头文件
- `kernel/arch` -> 平台代码(虽然暂不支持其他架构哈)


# 前言

由于一些过往的问题，前几章使用 **Intro** 前缀.
整个项目从 **工作区整理** 开始:

```
[bili] 000 工作区整理
           --The origin of this repo,workspace tidy up
```

# TextOS

要做什么? 实现一个操作系统!

实现一个 Uefi 引导的小操作系统

## Feature

使用一体化的架构,附带UdkDebugger(之后的提交)

## SigmaBoot

用于引导 TextOS

在我这里,它应该包含这些功能:

- 配置文件解析
- 显示分辨率设置
- Logo显示
- 内核运行资源文件加载
- 内核加载
- 内核PML4初始化
- 进入内核

## TextOS Kernel

我们首先先实现一个`PrintK`及Console相关的.

- 显示
- 内存管理
- 中断
- 系统调用
- 进程
- 文件系统
- ......

# Documents

详见 [README.md](../README.md#参考资料)

# Get Started!

使用 `make -C Src qemu` 来编译运行.

# Other

项目正在整理...QwQ

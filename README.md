# TextOS

这是一个 Uefi 引导的操作系统项目.

- Boot : SigmaBoot
- Opearting System : TextOS

- [Gitee | canyan233](https://gitee.com/canyan233)
- [GitHub | ljQAQ233](https://github.com/ljQAQ233)
- [Bilibili | maouai233](https://space.bilibili.com/503518259)

在B站不定期更新,有错误请提交issue,谢谢!学生党,更新慢,请见谅.

# 目录结构

与一般的目录结构大不相同...

- (Build 构建&输出)
- Docs 文档
- Src
   - Base    开发资源
   - Boot    基于EDKII项目改,使用Makefile
   - Config  Makefile配置
   - Include 头文件目录
      - Boot SigmaBootPkg的头文件
   - SigmaBootPkg Boot源码
   - Utils   工具

---

使用 Makefile 硬构强力驱动 /doge [[make]](#make)

# make

- `make -C Src qemu` -> running on Qemu
- `make -C Src qemubg` -> start uefi debugging
- `make -C Src compile_commands.json` -> AutoGen CompileCommands

---

Qemu启动可选项:

- QEMU_GPY
   - false -> 不显示Qemu图形化窗口

# 学习 / 开发此项目

## 初始化

```shell
sudo cp Src/Utils/Udk/udkdebugger.conf /etc
```

## compile_commands.json

```shell
make -C Src compile_commands.json
```

将 `Src/Utils/compile_commands.json`添加到配置中.

VSCode可以配置`includePath`

# 参考软件版本

- Provide
   - EDKII - vUDK2018
- Private
   - GCC - `gcc 7.5.0`
   - GDB - `gdb 8.1.1` (with `expat`)
   - QEMU - `QEMU emulator version 3.0.0`

# 参考资料

## 文档

- Docs

- [Intel® 64 and IA-32 Architectures Software Developer’s Manual](./Docs/Intel.pdf)
- [Unified Extensible Firmware Interface (UEFI) Specification Version 2.9 March 2021](./Docs/Docs/UEFI_Spec_2_9_2021_03_18.pdf)

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

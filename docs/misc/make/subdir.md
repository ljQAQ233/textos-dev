# 子模块路由

`config/subdir.mk` 提供 `@` 语法将目标分发到嵌套子模块.

## syntax

| 命令                       | 效果 (示意)           |
| -------------------------- | --------------------- |
| `make -C src app@clean`    | `make -C app clean`   |
| `make -C src app/lua@test` | 多跳递归到 `app/lua`  |
| `make -C src @target`      | 当前层执行 `target`   |

不含 `@` 时不触发, 普通构建不受影响.

## usage

```makefile
$(SUBMODS):
	$(MAKE) -C $@
include config/subdir.mk
```

可选: 在 include 前设 `_SUBDIR_PRE` 声明前置依赖.

## help

自动获得 `help` 目标列出本层可用目标. 配合 `@` 查看任意层级:

```
make help          # 顶层
make app@help      # app/ 层
```

## dive deeper

找到 `MAKECMDGOALS` 中含 `@` 的词, 吃掉路径第一段, 余下递归传给子 make.
工具链变量 (`CC`, `ARCH`, `ROOT` 等) 通过 `$(MAKE)` 自动继承. belike:

- `A/B/C@D`
- `B/C@D`
- `C@D`
- `D`

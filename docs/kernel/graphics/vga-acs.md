# VGA ACS 字符集

vga 文字模式下, 常用的标准字体是 IBM PC Code Page 437 (CP437).

这个是 可以换的, 但是默认的就很好了

---

console 控制台需要支持 特殊的字形, 需要解析 终端 的转义序列

- shift out - 切换到 acs 模式 - alternative character set
- shift in - 切换到常规 字符模式

在 ACS 模式下, 特定的字符意味着特定的符号, e.g.

```
$ man 3 curs_addch
          ACS      acsc
Symbol    Default  char   Glyph Name         Kitty
───────────────────────────────────────────────────
ACS_BLOCK #        0      solid square block  █
ACS_BOARD #        h      board of squares    ░
```

也就是说, 如果需要打印 `ACS_BLOCK`, 只需要 `shift out` 再打印 `0` 就好了

由于不同的 terminal emulator 提供的 `acsc char` 不同, 这里借助 `ncurses` 进行打印

- ncurses - [acsn.c](./acsn.c) - `deepseek-v4.1-flash-1017`
- TERM=linux - [`src/app/test/bin/acs.c`](../../../src/app/test/bin/acs.c) - 无 ncurses 版本

`TERM=linux` 的代码 用简单的 strace + vim 技巧就可以得到, 作为一份无 ncurses 的版本. 当然你需要在 linux console 里面运行 `(￣^￣)`

ncurses 运行之前 **一定** 要加上 `TERM=linux`, 其中, `linux` 是 linux console 的 terminfo, 我就直接用它的 terminfo 来写 textos 的 控制台了.

主要是简洁 (我感觉, 毕竟爱屋及乌?)

---

- 终端参考转义字符: <https://xtermjs.org/docs/api/vtfeatures/>
- 对照表: <https://www.ascii-codes.com/>
- 有意思的: <https://ascii.larkenx.me>
- 现成的 `ACS_` 对照表 <http://frankcox.ca/articles/ncurses-extended-characters/index.html>


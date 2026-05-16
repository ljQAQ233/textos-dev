# LAZY

prototypes 有点多, 我当然不想手动写...

```shell
universal-ctags --_xformat="%{typeref} %{name}%{signature}" -x asm.c | \
    cut -d':' -f2- | \
    sed 's/$/;/' |
    sed 's/restrict//g' |
    sed 's/const//g' > protos.c
```

然后 neovim 出场就很舒服了 (不用 sed 是因为可视化).

```vim
:%s/\s\zs\(\w\+\)\ze[,)]/,\1/g
:%s/ /,
:%s/()//g
:%s/(/,/g
:%s/)//g
```

修正一下用 **clang-format** 格式化就好了


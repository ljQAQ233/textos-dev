# puts / putchar

可以简单测试一下

```sh
cd test
make nbuf.out
strace ./nbuf.out
```

```
write(1, "xyz", 3xyz) = 3
write(1, "x", 1x)     = 1
write(1, "y", 1y)     = 1
write(1, "z", 1z)     = 1
```


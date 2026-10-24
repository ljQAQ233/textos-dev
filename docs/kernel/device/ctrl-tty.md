# 控制终端

> controlling terminal

- `man 4 tty`
- <https://pubs.opengroup.org/onlinepubs/9699919799.2008edition/basedefs/V1_chap03.html#tag_03_115>
- <https://linuxdev.cc/article/a0hduu.html>
- <https://labex.io/lesson/controlling-terminal?learn=true> (好玩的)

```shell
ps -o pid,tty
```

```
PID TT
   4187 pts/1
   4957 pts/1
```

---

**session leader** open tty 设备 时 (肯定不包括 `/dev/tty` 啦), 若未指定 `O_NOCTTY` 标志, 内核就会将该终端绑定为该会话的 controlling tty.

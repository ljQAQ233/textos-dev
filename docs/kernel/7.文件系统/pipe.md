# overview

```
int pipe(int fds[2]);
```

- fds[0] - rx
- fds[1] - tx

# vfs

依然使用 node 作为底层, 实现 node 的操作, 就可以实现 pipe

# todo

- `BLOCK`
- `NONBLOCK`

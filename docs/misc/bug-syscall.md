# stack switching

- 2024/01/20

> "小概率" 事件, 但是 大概率发生

在 `msyscall_handler` 被调用时, 中断还处于开启状态, 如果在切换到 内核栈 之前发生了 时钟中断, 随着任务切换, `cr3` 的改变, 原来储存在栈上的数据就失效了, 这时继续运行就会发生 `#PF` / `#UD`

禁用中断即可.

---

linux:

```
entry_SYSCALL_64
-> do_syscall_64
-> syscall_enter_from_user_mode
-> local_irq_enable
```


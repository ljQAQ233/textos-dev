# lai

> [Lightweight AML Interpreter](https://github.com/managarm/lai)

# init

需要内核提供的函数见 [wiki](https://github.com/managarm/lai/wiki/Host-API-Documentation).

TextOS 实现的函数见 `kernel/lai/layer.c`

其中, 很重要的一个:


```
/* Returns the (virtual) address of the n-th table that has the given signature,
   or NULL when no such table was found. */
void *laihost_scan(char *sig, size_t index);
```

---

在准备好了之后调用:

```c++
lai_create_namespace();
lai_enable_acpi(1); // 1 表示 ioapic 模式
```

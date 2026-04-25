# Dertermination of Access Rights

- CPL < 3 -> Supervisor-mode access
- CPL = 3 -> User-mode access

> CPL - Current Privilege Level 当前特权级
> 取决于 CPU 当前处于哪一个 代码段, CPL 与 当前段的段选择子中 RPL 保持一致 [^val]

[^val]: 5.5 PRIVILEGE LEVELS <br> `Normally, the CPL is equal to the privilege level of the code segment from which instructions are being fetched.`

![Segment Selector](images/seg-selector.png)

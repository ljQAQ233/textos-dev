# Calling Conventions

> 调用约定 (x64)

**汇编** 要与 **C语言** 交互就必须遵守这个约定

## 传参

- user : `rdi` -> `rsi` -> `rdx` -> `rcx` -> `r8` -> `r9` -> 栈 (反向存)
- syscall : `rdi` -> `rsi` -> `rdx` -> `r10` -> `r8` -> `r9`
    - `rax` - number

## 返回

- 64 bits - `rax` 保存 返回值
- 128 bits - `rdx` 保存 高64位

## 保留

调用方保存:
 - `rax`
 - `rdi`
 - `rsi`
 - `rdx`
 - `rcx`
 - `r8`
 - `r9`
 - `r10`
 - `r11`

被调用方保存: 
 - `rbx`
 - `rsp`
 - `rbp`
 - `r12`
 - `r13`
 - `r14`
 - `r15`

什么意思?

比如: 调用函数的一方使用的寄存器可能被 其所调用函数 修改, 所以在 进入前后 需要保存一些寄存器, 这些寄存器由 调用者 与 被调用者分工保存.

再如: 如果我的 `r11` 中保存了重要的信息, 那么调用函数之前 需要 先 `push r11`
再如: 如果我的 `r15` 中保存了重要的信息, 那么调用函数之前 不需要 保存. 但是对于被调用的函数来说, 如果有改变 `r15` 的值, 就需要有 保存和恢复 的步骤

---

可能说的比较绕, 编码时记住:

- 调用前 保全自己 - (保护好 "调用方保存" 中必要的寄存器)
- 修改时 顾及他人 - (保护好 "被调用方保存" 中必要的寄存器)

---

这个比较坑,  要不然可能出现一些奇奇怪怪的问题...

## Red zone!!!

> Signal handlers are executed on the same stack, but 128 bytes known as the red zone is subtracted from the stack before anything is pushed to the stack. This allows small leaf functions to use 128 bytes of stack space without reserving stack space by subtracting from the stack pointer. The red zone is well-known to cause problems for x86-64 kernel developers, as the CPU itself doesn't respect the red zone when calling interrupt handlers. This leads to a subtle kernel breakage as the ABI contradicts the CPU behavior. The solution is to build all kernel code with -mno-red-zone or by handling interrupts in kernel mode on another stack than the current (and thus implementing the ABI).    -- osdev
> For leaf-node functions (functions which do not call any other function(s)), a 128-byte space is stored just beneath the stack pointer of the function. The space is called the red zone. This zone will not be overwritten by any signal or interrupt handlers. Compilers can thus use this zone to save local variables. Compilers may omit some instructions at the starting of the function (adjustment of RSP, RBP) by using this zone. However, other functions may overwrite this zone. Therefore, this zone should only be used for leaf-node functions. gcc and clang offer the -mno-red-zone flag to disable red-zone optimizations.    -- wikipedia

不会调用其他函数的函数叫做 **leaf-node function** [^fanyi]. 这些函数的栈指针以下有一个占据 128 bytes 的特殊区域, 叫做 **"红区"** (red zone), 这个区域不会将 **不会**(假设) 任何 **信号/中断处理程序** 所**覆写**. 编译器使用这块区域来存储 **局部变量**. 编译器可能偷懒, 不去 调整 `rbp / rsp`. 比如 你申请了一个 64 字节的 char 数组, 编译器可能不会调整 `rsp -= 64` , 而是直接寻址, 它认为 此 红区 不会被修改. 实际上, 这对于内核的编写不是很友好, 例如 措不及防的中断, 假如 刚好遇到一个红区, 后来的事你就脑补吧. **('ヮ')**

[^fanyi]: 没有找到好翻译, 叶节点函数, 调用栈尾部的函数

红区可以通过编译选项 (gcc / clang), `-mno-red-zone` 来关闭

不关是 **有可能** 会 page fault 的, <strike>别问我怎么知道的</strike>

## 参考

- [System V ABI | osdev](https://wiki.osdev.org/System_V_ABI)
- [X86 calling conventions | wikipedia](https://en.wikipedia.org/wiki/X86_calling_conventions)


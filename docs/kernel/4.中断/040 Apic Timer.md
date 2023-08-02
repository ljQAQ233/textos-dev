# Overview

**Building... This is a preview version**

---

在传统的操作系统上, 使用的是 PIT (Programmable Interval Timer),
我们使用 Apic 附带的定时器 -> APIC Timer

与 PIT 相比, 它具有更高的精度, 且与 **Local APIC** 一起集成在每个 CPU 的内部, 也就是每个 CPU 都可以有不同的定时器中断, 相比来说更加灵活.

## Options

我们可以调整 Apic Timer 的模式:

|模式|值|说明|
|------------|------|--------|
|One shot    |`0b00`|只执行一次|
|Periodic    |`0b01`|周期执行 |
|TSC deadline|`0b10`| -      |

## Initialize

> Why?

每个 CPU 的频率各不相同, 因此我们不能直接写死,
如果使用 CPUID 的指令, 那么只能获取出厂设定的频率,
如果 CPU 被超频了,就自然不管用了.

---

需要一个基准, 可以是它们:

- PIC
- RTC
- ...

我们使用 RTC 来进行初始化:

在不使用中断的情况下,我们在启动定时器后需要不断读取定时器的当前计数, 直到它归零

```c++
```

## Start here!

1. 关闭定时器中断
```c++
LAPIC_SET(LAPIC_TM, S_TM(TM_ONESHOT, 0) | LVT_MASK); // 屏蔽 Apic Timer 的本地中断
```
2. 设置默认因子
```c++
LAPIC_SET(LAPIC_DCR, 0b0000);                        // 设置 除数 (因子) 为 2
```
3. 设置初始计数到最大
```c++
LAPIC_SET(LAPIC_TICR, 0xFFFFFFFF);                   // 设置 初始计数到最大 (-1)
```
4. PIT 的傻瓜睡眠 n ms
```c++
pit_sleepms (10);
```
5. 在 n ms 内, 计算写入初始数值
```c++
LAPIC_SET(LAPIC_TICR, 0xFFFFFFFF - LAPIC_GET(LAPIC_TCCR)); // 计算 100ms 的 ticks
LAPIC_SET(LAPIC_TM, S_TM(TM_PERIODIC, INT_TIMER));         // 步入正轨, 每 100ms 产生一次时钟中断
```
6. 注册中断函数
```c++
intrregister (INT_TIMER, timer_handler);                    // 注册中断函数
```

---

NOTE: 由于是 Apic 的本地中断, 所以我们无需向 IOApic 注册.

## After all

我们可以用中断函数做一些什么事?

- 任务
- 软中断 (?)

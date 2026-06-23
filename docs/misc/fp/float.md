# double

> Double-precision floating-point format, FP64 or float64
> 
> 在 *IEEE754* 中, 64位 以 2 为底 的格式成为 **binary64**, 后绪在 *IEEE754-1985* 中称作 **double**

---

一个 `x86_64` 下的 double 占用 8 字节, 它可以表示大约 `[2.22507385850720138309e-308, 1.79769313486231570815e+308]` 的数字.

uint64_t 也是 8 字节, 但是却比这个数字少得 多得多. 通过一下实验可以看到 `1e300` 的不精确表示:

```c++
#include <stdio.h>

int main()
{
    return printf("%f\n", 1e300);
    return 0;
}
```

主流设计采用 **IEEE754** 标准:

- S - 1 bit - sign bit
- E - 11 bit - exponent
- F - 52 bit - fraction

E 是 **biased exponent**

$$
(-1)^{S} (1.b_{51}b_{50}\dots b_0)_2 \times 2^{E-1023}
$$

其中, $1023$ 被称为 **exponent bias**

$$
(-1)^{\text{S}}\left(1+\sum _{i=1}^{52}b_{52-i}2^{-i}\right)\times 2^{E-1023}
$$

## 指数表述

- $0 \le E \le {7ff}_{16} = 2047$
  - $E = 0$ - Signed Zero
  - $1 \le E \le 2046$ - Normal numbers
  - $E = 2047$
    - $F = 0$ - Inf
    - $F \neq 0$ - NaN

## 试验

使用 **python** 的大整数除法可以初步验证, 变形:

$$
(-1)^{\text{S}}\left(2^{52}+\sum _{i=0}^{51}b_{i}2^{i}\right)\times \frac{2^{E-1023}}{2^{52}}
$$

- [float.py](float.py)

## 舍入

计算好像一个大整数除法就结束了, 最难的部分 (据说), 是舍入. 但是对于 libc 而言, 标准 并没有要求精确舍入 ( ￣ー￣)σ

## 建模

### `big_pow2` 预处理表

假设 $e = 10 = (1010)_2 = 2^1 + 2^3 $

### `big_div` 小数位数

设 $y = \frac{x}{2^e}$ ($x$ 为变量, $0 \le x \le 2^e$), 求 y 的小数点后最多有几位小数?

> $\frac{1}{2^e} = \frac{5^e}{10^e} \Rightarrow y = \frac{x \cdot 5^e}{10^e}$, 则 $y$ 小数点后最多有 $e$ 位小数

# references

- <https://en.wikipedia.org/wiki/Double-precision_floating-point_format>
- 高精度除法 - <https://oi-wiki.org/math/bignum/#%E9%99%A4%E6%B3%95>


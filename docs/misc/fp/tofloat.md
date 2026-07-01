# preface

本书, 先以工程的角度实现 `strtod`, 然后再从算法角度理解 程序 的正确性

## conventions

将 `nptr` 视为原始字符串

# hex2fp

满足 `0xHHHH.hhhh[Pp][+-]e`[^1] 的格式, `H` 与 `h` 可省略其中一个

[^1]: `HHHH` 代表 一个或者多个 hex 数字.

如果 这个数字不能被一个 浮点类型 所表示, 会在十六进制中采取特定的截断方式. 我本以为这个方式很特别, <strike>比如有规律地采样等等, 然而很简单</strike>

要使得这个方式获得的小数最逼近 `nptr`, 高位不能舍弃, 也就要求 **高位截断**

hex 如果形式得当, 就是在描述最本真的 浮点类型, 也就意味着不会发生舍入. 但是 由于 **截断** 可能存在, 截断的一部分天然提供了 **舍入** 的条件.

---

如果 `nptr` 是 规范化的表达 (`0x1.hhhh[Pp][+-]e`), `e` 就是 `unbiased`, `hhhh` 则是 `mantissa`, 这是 `printf` 告诉我们的

当然 `nptr` 可以代表 `0`

难点在于, 我们需要处理 **非规范数** 以及 `1.` 不是 `1.` 而是 `HHHH`[^1] 的情况, 这时需要进行归一化

将 `HHHHhhhh` 作为整体, 作为整数抽离同时记录小数点位置之后, 抽出 **MSB** 作为 **implicit bit (1)**, 后续部分就是 **mantissa**, 此过程需要对 e 做出调整

# dec2fp

- 符号位 - `sign`
- 整数有效数 - `M`
- 十进制指数 - `E` (`e10`)
- 输入的纯数字部分 - `V`

$$
V = (-1)^s \cdot M \cdot 10^E
$$

$$
\lvert V \rvert = M \cdot 10^E = M \cdot 2^E \cdot 5^E
$$

其中, $2^E$ 可以直接算作 **二进制偏移** 加入 **biased** 中

## E >= 0 时

V 有准确值, 使用一次 高精度乘法 后 高位截断 即可.

## E < 0 时

$10^{-E}$ 位于分母, 即便是高精度除法, 除完之后存在小数, 还需要换为二进制才能 **约取**, 且小数处理不便, 这时 经典算法 依旧提取 $2^E$, 在原有精度上缩放 $2^K$ (K 未知)

令 $Q = -E$

$$
\lvert V \rvert = \frac{M \cdot 2^{E}}{5^{-E}} = \frac{M \cdot 2^K}{5^{Q}} \cdot 2^{-Q-K}
$$

令 $N = {M \cdot 2^K}$

$$
\frac{N}{5^Q} = \mathit{quo} \cdot 5^Q + \mathit{rem}
$$

`quo` & `rem` 为整数, quo 处理之后即为 mantissa

---

此时取 `K` 即可.

`K` 的作用是让 `quo` 足够大, 使其二进制展开能提供完整的 `G/R/S` 位 (`Guard / Round / Sticky`) 供舍入使用:

以 `binary64` 为例:

- $1\sim 53$ 位: significand (mantissa)
- 54 位: **G** (guard bit)
- 55 位: **R** (round bit)
- 56 位及之后 **或上** 余数 ($rem \neq 0$): **S** (sticky bit)

因此要求 $\text{bit\_length}(\mathit{quo}) \ge 55$

$quo = \lfloor \frac{N}{5^Q} \rfloor$ 的位宽近似为:

$$
\text{bit\_length}(\mathit{quo}) \approx \log_2 M + K - Q \cdot \log_2 5
$$

代入条件:

$\log M + K - Q \log 5 \ge 55$

$\Longrightarrow K \ge 55 + Q \log 5 - \log M \ge 55 + \lceil Q \cdot \log_2 5 \rceil$

# round-to-nearest-even

- <https://stackoverflow.com/questions/8981913/how-to-perform-round-to-even-with-floating-point-numbers>

```c++
// G R S   ULP
// 0 * * < 0.5 -
// 1 0 0 = 0.5 +
// 1 1 * > 0.5 +
// 1 0 1 > 0.5 +
```


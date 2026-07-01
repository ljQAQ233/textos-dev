from bin64 import *
from math import log2

s = input()
control = float(s)
int_part, _, dec_part = s.partition('.')

# V = M x 10 ^ -Q, Q 为整数 >= 0
M = int(int_part + dec_part)
Q = len(dec_part)
print(f"int = {int_part}")
print(f"dec = {dec_part}")
print(f"M = {M}, Q = {Q}")

# Emin < E <= Emax
found = False
pow10_Q = 10 ** Q
for E in range(Emin, Emax + 1):
    e = E - Ebias
    if pow10_Q * (2 ** e) <= M and M < pow10_Q * (2 ** (e + 1)):
        found = True
        break

C_S, C_E, C_F = xdouble(control)
assert found
assert e == C_E - Ebias

# 粗糙计算
# F = M * (2 ** (FRACBITS - e)) / (10 ** Q) - (2 ** FRACBITS)
print(f"{e=}")
print(f"     {M * (2 ** (FRACBITS - e)) / (10 ** Q) - (2 ** FRACBITS):.100f}")

pow5_Q = 5 ** Q
K = FRACBITS + 2 + int(Q * log2(5)) + 64

# 放大 2^K
SHIFT = K - Q - e
if SHIFT >= 0:
    quo = (M << SHIFT) // pow5_Q # => floor(N * 2^K), 1 <= N < 2
    rem = (M << SHIFT) %  pow5_Q
else:
    quo = M // (pow5_Q << (-SHIFT))
    rem = M %  (pow5_Q << (-SHIFT))

sig = quo >> (K - FRACBITS)
F = sig - (1 << FRACBITS)
round_bit = (quo >> (K - FRACBITS - 1)) & 1
sticky = (quo & ((1 << (K - FRACBITS - 1)) - 1)) > 0 or rem > 0

# round-to-nearest-even
if round_bit and (sticky or (F & 1)):
    F += 1
if F >= (1 << FRACBITS):
    F = 0
    e += 1

E = e + Ebias
print(E, F)
print(C_E, C_F)

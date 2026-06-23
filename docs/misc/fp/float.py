import sys
from math import log2, ceil
from bin64 import *

sys.set_int_max_str_digits(10000)

STRIDE = 128

# do not edit those constants!!!
MAXPOW = max(Emax - Ebias - FRACBITS, abs(1 - Ebias - FRACBITS))

if 1:
    print("the length of power of 2:")
    last = -STRIDE
    for i in range(0, MAXPOW):
        r = 2 ** i
        l = len(str(r))
        if l - last >= STRIDE:
            last = l
            print(f"{{ {i}, {l} }},")
    print("=============")

if 1:
    sum = 0
    print("the pow of the pow of 2:")
    for i in range(0, ceil(log2(MAXPOW)) + 1):
        r = 2 ** (2 ** i)
        s = str(r)
        sum += len(s)
        rhex = "".join(f"\\x{((b - 0x30) & 0xFF):02x}" for b in s.encode())
        print(f"[{i}] = {{ {len(s)}, \"{rhex}\" }},")
    print(f"// sum = {sum}")
    print("=============")

# python uses binary64 as float
d: float = 3.1415926
d: float = 1e300

S, E, F = xdouble(d)
e = E - 1023
s = -1 if S else 1
print(f"d = {d}")
print(f"sign bit = {S} ({s})")
print(f"exponent = {E} (e={e})")
print(f"fraction = {F}")

# multiply an offset to keep it an int
off = (10 ** 200)
exp = E - 1023 - 52
num = s * (2 ** 52 + F) * off
if exp >= 0:
    R: int = num * (2 ** exp)
else:
    R: int = num // (2 ** (-exp))
print(R)

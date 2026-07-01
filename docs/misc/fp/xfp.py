from bin64 import xdouble

d = float.fromhex(input())

S, E, F = xdouble(d)
e = E - 1023
s = -1 if S else 1
print(f"d = {d}")
print(f"sign bit = {S} ({s})")
print(f"exponent = {E} (e={e})")
print(f"fraction = {F}")

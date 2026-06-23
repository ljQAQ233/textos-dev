import struct

# x87 fpu
# MAXFRACBITS = 64
# MAXEXPOBITS = 14

# binary64
FRACBITS = 52
MAXEXPOBITS = 11

Einfnan = 2 ** MAXEXPOBITS - 1
Esubnorm = 0
Emax = 2 ** MAXEXPOBITS - 1 - 1
Emin = 1
Ebias = 2 ** (MAXEXPOBITS - 1) - 1

emax = Emax - Ebias
emin = 1 - emax

def xdouble(x):
    data = struct.pack('>d', x)
    n = int.from_bytes(data, 'big')
    S = n >> 63
    E = (n >> 52) & 0x7ff
    F = n & 0xFFFFFFFFFFFFF
    return S, E, F

def mkdouble(S, E, F):
    []
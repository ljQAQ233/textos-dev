# arp

> Address Resolution Protocol
>
> IPv4 -> MAC

```c++
struct arp
{
    u16 htype;
    u16 ptype;
    u8  hlen;
    u8  plen;
    u16 opcode;
    u8  srchw[hlen];
    u8  srcpr[plen];
    u8  dsthw[hlen];
    u8  dstpr[plen];
};
```

## len

对于 eth :

- `hlen = 6`
- `plen = 4`

## opcode

- REQUEST - 查询对方
- REPLY - 回复查询

# warning

转换为大端字节序

# todo

- arp cache

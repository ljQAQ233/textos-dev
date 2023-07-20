
# x2APIC

- `IA32_APIC_BASE`
  - `[EN]` - 使能 apic
  - `[EXTD]` - 使能 x2apic

4 种状态[^stat]:

- apic 关闭 : `[EN]=0` `[EXTD]=0`
- xapic 开启 : `[EN]=1` `[EXTD]=0`
- x2apic 开启 : `[EN]=1` `[EXTD]=1`
- 无效 (`#GP`) : `[EN]=0` `[EXTD]=0`

[^stat]: `10.12.5.1 x2APIC States`

## x2APIC 地址空间

> 10.12.1.2 x2APIC Register Address Space


# Fat32

root_count = 0
root_cluster = 2

fat_size = 252
fat_tabnum = 2

sector_reserved = 32
sector_size = 512
sector_num  = 32768
sectors_per_cluster = 1

## calculate

### root dir

> `root_dir_sectors = ((root_count * 32) + (sector_size - 1)) / sector_size`

root_dir_sectors = (0 + 511) / 512 = 0 : int

> `first_data_sector = sector_reserved + (fat_tabnum * fat_size) + root_dir_sectors`

first_data_sector = 32 + 2 * 252 + 0 = 536

> `first_sector_of_cluster = ((cluster - 2) * sectors_per_cluster) + first_data_sector`

root_sector = ((2 - 2) * 1) + 536 = 536

+ offset = root_sector * sector_size = 0x43000

### get into `/EFI/` !

```
00043000  45 46 49 20 20 20 20 20  20 20 20 10 00 48 dd 52  |EFI        ..H.R|
00043010  2e 58 2e 58 00 00 dd 52  2e 58 03 00 00 00 00 00  |.X.X...R.X......|
```

cluster_low = 0x0003
cluster_high = 0x0000

cluster = 0x0003

+ offset = (((3 - 2) * 1) + 536) * 512 = 0x43200


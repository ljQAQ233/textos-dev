#pragma once

struct blk_part
{
    u8 bootable; // bootable (active) -> 0x80
    u8 start_head;
    u16 start_sec : 6;
    u16 start_clinder : 10;
    u8 sysid;
    u8 end_head;
    u16 end_sec : 6;
    u16 end_clinder : 10;
    u32 relative;
    u32 total;
} _packed;

struct blk_part_info
{
    addr_t ptoff;
    addr_t ptend;
    int sysid;
    struct blk_part record;
};

int blk_part_scan(devst_t *blkdev);
int blk_part_register(devst_t *blkdev, struct blk_part_info *info,
                      int scan_idx);

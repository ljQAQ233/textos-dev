#pragma once

struct blk_mbr
{
    u8 others[446];
    u8 ptab[4][16];
    u16 endsym;
} _packed;

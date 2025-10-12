#pragma once

/* allocate `num` physical pages */
addr_t pmm_allocpages(size_t num);

/* hardcore! */
void pmm_allochard(addr_t pa, size_t num);

/* free physical pages */
void pmm_freepages(addr_t pa, size_t num);

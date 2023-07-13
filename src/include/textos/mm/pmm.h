#pragma once

/* allocate `num` physical pages */
void *pmm_allocpages (size_t num);

/* hardcore! */
void pmm_allochard (void *page, size_t num);

/* free physical pages */
void pmm_freepages (void *page, size_t num);
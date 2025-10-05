/*
 * make temporary pages mapping to virtual mem space.
 * a pvpage's size is assigned to PAGE_SIZE, the size of a page frame.
 */
#pragma once

int make_pvpage(addr_t phy, void *res);

void break_pvpage(void *page);

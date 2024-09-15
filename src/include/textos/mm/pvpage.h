#pragma once

int make_pvpage(addr_t phy, void *res);

void break_pvpage(void *page);

#pragma once

#include <textos/mm/map.h>

int vmm_caadjust(addr_t *vrt);
void *vmm_phyauto(addr_t vrt, size_t num, int flgs);
void *vmm_allocvrt(size_t num);
void *vmm_allocpages(size_t num, int flgs);
void vmm_allocpv(size_t num, addr_t *va, addr_t *pa);

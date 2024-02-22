#pragma once

#include <textos/mm/map.h>

/* Check if the Vrt is a canonical format
   vrt addr and adjust it if it is invalid

   @retval  int   The state */
int vmm_caadjust (u64 *vrt);

/* Link virtual pages that Vrt points, 
   Num holds with an real physical pages */
void *vmm_phyauto (u64 vrt, size_t num, u16 flgs);

void *vmm_allocpages (size_t num, u16 flgs);


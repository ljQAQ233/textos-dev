#ifndef CRYPT_DES_H
#define CRYPT_DES_H

#include <stdint.h>

struct expanded_key {
  uint32_t l[16], r[16];
};

void __des_setkey(const unsigned char *key, struct expanded_key *ekey);
void __do_des(uint32_t l_in, uint32_t r_in,
    uint32_t *l_out, uint32_t *r_out,
    uint32_t count, uint32_t saltbits, const struct expanded_key *ekey);

#endif

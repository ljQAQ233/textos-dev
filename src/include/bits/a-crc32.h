#define __NEED_size_t
#define __NEED_uint8_t
#define __NEED_uint32_t
#include <bits/alltypes.h>

#define CRC32C_POLY 0x82F63B78u /* reversed 0x1EDC6F41 */

static inline uint32_t crc32c_update(uint32_t crc, const void *buf, size_t len)
{
    const uint8_t *p = (const uint8_t *)buf;

    while (len--) {
        crc ^= *p++;
        for (unsigned k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (CRC32C_POLY & (0u - (crc & 1u)));
    }
    return crc;
}

static inline uint32_t crc32c_final(uint32_t crc)
{
    return crc ^ 0xFFFFFFFFu;
}

static inline uint32_t crc32c(const void *buf, size_t len)
{
    return crc32c_final(crc32c_update(0xFFFFFFFFu, buf, len));
}

static inline uint32_t crc32c_str(const char *s, size_t len)
{
    uint32_t crc = crc32c_update(0xFFFFFFFFu, s, len);
    uint8_t sz[sizeof(size_t)];

    for (size_t i = 0; i < sizeof sz; i++)
        sz[i] = (uint8_t)(len >> (8 * i));

    crc = crc32c_update(crc, sz, sizeof sz);
    return crc32c_final(crc);
}

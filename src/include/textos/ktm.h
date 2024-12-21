#ifndef __KTM_H__
#define __KTM_H__

typedef struct
{
    size_t s, t;
    char *label;
} ktm_t;

void ktm_start(ktm_t *k, char *label);

void ktm_stop(ktm_t *k);

void ktm_dump(ktm_t *k);

#define KTM_START(label)    \
        ktm_t _ktm_##label; \
        ktm_start(&_ktm_##label, #label); \

#define KTM_END(label)           \
        ktm_stop(&_ktm_##label); \
        ktm_dump(&_ktm_##label); \

#endif
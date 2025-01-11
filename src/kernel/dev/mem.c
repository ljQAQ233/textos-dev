#include <textos/dev.h>

#include <string.h>

// EOF
static int null_read(dev_t *dev, void *buf, size_t cnt)
{
    memset(buf, EOF, cnt);
    return 0;
}

// drop!!!
static int null_write(dev_t *dev, void *buf, size_t cnt)
{
    return 0;
}

// 0
static int zero_read(dev_t *dev, void *buf, size_t cnt)
{
    memset(buf, 0, cnt);
    return cnt;
}

// drop
static int zero_write(dev_t *dev, void *buf, size_t cnt)
{
    return 0;
}

// todo : full

static dev_t null = {
    .name = "null",
    .read = null_read,
    .write = null_write,
    .type = DEV_CHAR,
    .subtype = DEV_NULL,
};

static dev_t zero = {
    .name = "zero",
    .read = zero_read,
    .write = zero_write,
    .type = DEV_CHAR,
    .subtype = DEV_ZERO,
};

void __dev_initmem()
{
    // todo : major nr
    // null / zero
    dev_register(NULL, &null);
    dev_register(NULL, &zero);
}

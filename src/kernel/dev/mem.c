#include <textos/dev.h>
#include <textos/klib/string.h>

// EOF
static int null_read(devst_t *dev, void *buf, size_t cnt, ...)
{
    memset(buf, EOF, cnt);
    return 0;
}

// drop!!!
static int null_write(devst_t *dev, void *buf, size_t cnt, ...)
{
    return 0;
}

static dev_opts_t null_op = {
    .read = null_read,
    .write = null_write,
    .driver_name = "mem-null",
};

// 0
static int zero_read(devst_t *dev, void *buf, size_t cnt, ...)
{
    memset(buf, 0, cnt);
    return cnt;
}

// drop
static int zero_write(devst_t *dev, void *buf, size_t cnt, ...)
{
    return 0;
}

static dev_opts_t zero_op = {
    .read = zero_read,
    .write = zero_write,
    .driver_name = "mem-zero"
};

static dev_opts_t mem_op = {};

// todo : full

static devst_t null = {
    .name = "null",
    .type = DEV_CHAR,
    .subtype = DEV_NULL,
    .minor = 3,
    .op = &null_op,
};

static devst_t zero = {
    .name = "zero",
    .type = DEV_CHAR,
    .subtype = DEV_ZERO,
    .minor = 5,
    .op = &zero_op,
};

static devst_t mem = {
    .name = "mem",
    .type = DEV_CHAR,
    .subtype = DEV_MEMORY,
    .op = &mem_op,
};

void __dev_initmem()
{
    // todo : major nr
    // null / zero
    dev_register(NULL, &mem);
    dev_register(&mem, &null);
    dev_register(&mem, &zero);
}

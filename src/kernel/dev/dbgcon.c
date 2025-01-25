#include <io.h>
#include <textos/dev.h>

#define R_CON 0xe9

static int dbgcon_write(dev_t *dev, void *buf, size_t cnt)
{
    char *s = (char *)buf;
    for (int i = 0 ; i < cnt ; i++)
        outb(R_CON, s[i]);
    return cnt;
}

static dev_t dbgcon = {
    .name = "dbgcon",
    .write = dbgcon_write,
    .type = DEV_CHAR,
    .subtype = DEV_DBGCON,
};

void __dev_initdbgcon()
{
    dev_register(NULL, &dbgcon);
}
#include <textos/dev.h>

static devst_t anony = {
    .name = "anony",
    .type = DEV_NONE,
    .subtype = DEV_ANONY,
    .major = 0,
    .minor = 0,
    .subdev = LIST_INIT(anony.subdev),
};

static devstp_t pri = { &anony };

void dev_register_anony(devst_t *dev)
{
    dev->type = DEV_NONE;
    dev->subtype = DEV_ANONY;
    dev_register(&anony, dev);
}

void __dev_initanony()
{
    __dev_register(&pri);
}

#include <textos/printk.h>
#include <textos/assert.h>
#include <textos/ktm.h>
#include <textos/tick.h>

void ktm_start(ktm_t *k, char *label)
{
    ASSERTK(k != NULL);
    k->s = __ktick;
    k->label = label;
}

void ktm_stop(ktm_t *k)
{
    k->t = __ktick;
}

void ktm_dump(ktm_t *k)
{
    size_t sec = (k->t - k->s) / 100; // 10ms per tick
    dprintk(K_SYNC,
      "ktm : [%s] spend %us (%llu ~ %llu)\n",
      k->label, sec, k->s, k->t
      );
}

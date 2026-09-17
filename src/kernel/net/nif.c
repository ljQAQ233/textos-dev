#include <textos/errno.h>
#include <textos/klib/string.h>
#include <textos/net/socket.h>

int nif_ioctl(nif_t *nif, int req, void *argp)
{
    ifreq_t *ifr = argp;
    switch (req)
    {
    case SIOCGIFADDR:
        ip_addr_copy(ifr->addr, nif->ip);
        return 0;

    case SIOCSIFADDR:
        ip_addr_copy(nif->ip, ifr->addr);
        return 0;

    case SIOCGIFNETMASK:
        ip_addr_copy(ifr->addr, nif->netmask);
        return 0;

    case SIOCSIFNETMASK:
        ip_addr_copy(nif->ip, ifr->addr);
        return 0;

    case SIOCGIFBRDADDR:
        ip_addr_copy(nif->broadcast, ifr->broadaddr);
        return 0;
    
    case SIOCSIFBRDADDR:
        ip_addr_copy(ifr->broadaddr, nif->broadcast);
        return 0;

    case SIOCGIFHWADDR:
        eth_addr_copy(ifr->hwaddr, nif->mac);
        return 0;

    default:
        return -EINVAL;
    }

    return 0;
}

static list_t nifs = LIST_INIT(nifs);

void nif_register(nif_t *nif)
{
    list_insert(&nifs, &nif->nifs);
}

nif_t *nif_find(char *name)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &nifs)
    {
        nif_t *n = CR(ptr, nif_t, nifs);
        if (strcmp(n->name, name) == 0) return n;
    }

    return NULL;
}

nif_t *nif_find_byip4(ipv4_t ip, nif_t *fallback)
{
    list_t *ptr;
    LIST_FOREACH(ptr, &nifs)
    {
        nif_t *n = CR(ptr, nif_t, nifs);
        if (ip_addr_cmp(n->ip, ip)) return n;
    }
    return fallback;
}

nif_t *nif_find_default()
{
    return CR(nifs.prev, nif_t, nifs);
}

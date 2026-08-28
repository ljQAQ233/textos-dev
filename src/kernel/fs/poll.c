#include <textos/fs.h>
#include <textos/fs/poll.h>
#include <textos/task.h>

void fs_pollee_init(struct fs_pollee *pollee)
{
    list_init(&pollee->r_pollee);
}

void fs_poller_init(struct fs_poller *poller, int events,
                    struct fs_openctx *openctx)
{
    short defevents = POLLHUP | POLLERR | POLLNVAL;
    poller->info_events = events | defevents;
    poller->info_openctx = openctx;
    poller->info_poller = task_current();
    poller->revents = 0;
    poller->in_pollee_list = false;
}

void fs_poll_setup(struct fs_pollee *pollee, struct fs_poller *poller)
{
    poller->in_pollee_list = true;
    list_insert(&poller->info_openctx->r_pollers, &poller->l_of_openctx);

    pollee->npollers += 1;
    list_insert(&pollee->r_pollee, &poller->l_of_pollee);
}

// assert l_ctx_all is a node of r_pollee
void fs_poll_teardown(struct fs_pollee *pollee, struct fs_poller *poller)
{
    if (!poller->in_pollee_list) return;
    pollee->npollers -= 1;
    list_remove(&poller->l_of_pollee);
    list_remove(&poller->l_of_openctx);
}

bool fs_poll_deliver_to(struct fs_pollee *pollee, short revents,
                        struct fs_poller *poller, bool unblock)
{
    if (!(poller->info_events & revents)) return false;
    poller->revents = revents;
    if (unblock) {
        if (!poller->info_poller) return false;
        task_unblock(poller->info_poller, 0);
        poller->info_poller = NULL;
    }
    return true;
}

bool fs_poll_deliver(struct fs_pollee *pollee, short revents)
{
    if (!pollee->npollers) return false;
    list_t *ptr;
    LIST_FOREACH(ptr, &pollee->r_pollee)
    {
        struct fs_poller *poller = CR(ptr, struct fs_poller, l_of_pollee);
        if (fs_poll_deliver_to(pollee, revents, poller, true)) return true;
    }
    return false;
}

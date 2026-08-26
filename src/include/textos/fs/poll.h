#pragma once

#include <bits/poll.h>
#include <textos/klib/list.h>

struct fs_poller
{
    short info_events;
    struct fs_openctx *info_openctx;
    struct task *info_poller;
    short revents;
    bool in_pollee_list;
    list_t l_of_openctx; // link to fs_openctx
    list_t l_of_pollee;  // link to fs_pollee
};

struct fs_pollee
{
    size_t npollers;
    list_t r_pollee; // root of fs_pollee
};

void fs_pollee_init(struct fs_pollee *pollee);
void fs_poller_init(struct fs_poller *poller, int events,
                    struct fs_openctx *openctx);
void fs_poll_setup(struct fs_pollee *pollee, struct fs_poller *poller);
void fs_poll_teardown(struct fs_pollee *pollee, struct fs_poller *poller);
bool fs_poll_deliver_to(struct fs_pollee *pollee, short revents,
                        struct fs_poller *poller, bool unblock);
bool fs_poll_deliver(struct fs_pollee *pollee, short revents);

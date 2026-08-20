#pragma once

#include <bits/event.h>

struct task;
struct devst;

struct event_client
{
    struct task *waiter;
    struct event ev;
};

struct event_registry
{
    struct devst *evdev;
    struct event_client *client;
};

void event_deliver(struct event_registry *this);
void event_push_any(struct event_registry *this, struct event *ev);
void event_push_keyboard(struct event_registry *this, keysym_t sym);
void event_push_mouse(struct event_registry *this, keysym_t status, int dx,
                      int dy);

struct event_registry *event_register(enum event_type type);

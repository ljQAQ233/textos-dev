#pragma once

#include <bits/event.h>

devst_t *event_register(enum event_type type);
void event_deliver(devst_t *this);
void event_push_any(devst_t *this, struct event *ev);
void event_push_keyboard(devst_t *this, keysym_t sym);
void event_push_mouse(devst_t *this, keysym_t status, int dx, int dy);

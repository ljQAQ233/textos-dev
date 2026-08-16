typedef signed int keysym_t;

enum event_type
{
    EV_NONE,
    EV_KEYBOARD,
    EV_MOUSE,
    EV_MAXTYPE,
};

struct event_mouse
{
    int dx;
    int dy;
};

struct event
{
    enum event_type type;
    keysym_t sym;
    struct event_mouse m;
};


#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/keys.h>
#include <unistd.h>

void parse_none(struct event *ev)
{
    printf("event: none\n");
}

void parse_kbd(struct event *ev)
{
    printf("event: kbd\n");
    printf("type: %c\n", KEYTYP(ev->sym));
    printf("char: %c\n", KEYCHR(ev->sym));
    printf("state:");
    if (ev->sym & KEY_S_LSHIFT) printf(" lshift");
    if (ev->sym & KEY_S_RSHIFT) printf(" rshift");
    if (ev->sym & KEY_S_LALT) printf(" lalt");
    if (ev->sym & KEY_S_RALT) printf(" ralt");
    if (ev->sym & KEY_S_LCTRL) printf(" lctrl");
    if (ev->sym & KEY_S_RCTRL) printf(" rctrl");
    if (ev->sym & KEY_S_CAPSLK) printf(" capslk");
    if (ev->sym & KEY_S_LSUPER) printf(" lsuper");
    if (ev->sym & KEY_S_RSUPER) printf(" rsuper");
    if (ev->sym & KEY_S_APP) printf(" app");
    if (ev->sym & KEY_S_NUMLK) printf(" numlk");
    if (ev->sym & KEY_S_COMPOSE) printf(" compose");
    if (ev->sym & KEY_S_WAIT) printf(" wait");
    if (ev->sym & KEY_S_ERROR) printf(" error");
    printf("\n");

    if ((KEYCHR(ev->sym) == 'c') && // optional handling ctrl-c
        (ev->sym & (KEY_S_LCTRL | KEY_S_RCTRL)))
        exit(0);
}

void parse_mouse(struct event *ev)
{
    char pr[4] = "---";
    if (ev->sym & KEY_S_MOUSE_LEFT) pr[0] = 'l';
    if (ev->sym & KEY_S_MOUSE_MIDDLE) pr[1] = 'm';
    if (ev->sym & KEY_S_MOUSE_RIGHT) pr[2] = 'r';
    printf("event: mouse\n");
    printf("pressed: %s\n", pr);
    printf("dx: %d\n", ev->m.dx);
    printf("dy: %d\n", ev->m.dy);
    printf("dz: %d\n", ev->m.dz);
}

void (*parsers[EV_MAXTYPE + 1])(struct event *ev) = {
    [EV_NONE] = parse_none,
    [EV_KEYBOARD] = parse_kbd,
    [EV_MOUSE] = parse_mouse,
};

int main(int argc, char *argv[])
{
    int openfl = O_RDONLY;
    int interval = 0;
    if (argc < 2) {
        fprintf(stderr, "too few argument\n");
        return 1;
    }
    if (argc > 2) {
        openfl |= O_NONBLOCK;
        interval = atoi(argv[2]);
    }

    int fd = open(argv[1], openfl);
    if (fd < 0) {
        perror(NULL);
        return 1;
    }

    int ret;
    struct event ev;
    struct pollfd pfd = {fd, POLLIN};
    for (;;) {
        if (interval) {
            pfd.revents = 0;
            ret = poll(&pfd, 1, interval);
            if (ret < 0) goto err;
        }
        ret = read(fd, &ev, sizeof(ev));
        if (ret < 0) goto err;
        if (EV_NONE <= ev.type && ev.type < EV_MAXTYPE) {
            parsers[ev.type](&ev);
            printf("\n");
        }
    }

    return 0;
err:
    perror(NULL);
    return 1;
}

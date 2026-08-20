#include <bits/event.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <textos/dev/keys.h>
#include <unistd.h>

void parse_none(struct event *ev)
{
    printf("event: none\n");
}

void parse_kbd(struct event *ev)
{
    printf("event: kbd\n");
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

    struct event ev;
    for (;;) {
        int ret = read(fd, &ev, sizeof(ev));
        if (ret < 0) {
            perror(NULL);
            return 1;
        }
        if (EV_NONE <= ev.type && ev.type < EV_MAXTYPE) {
            parsers[ev.type](&ev);
            printf("\n");
        }
        if (interval) usleep(interval * 1000);
    }

    return 0;
}

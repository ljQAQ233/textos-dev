#pragma cone

typedef ssize_t (*tty_iosop_t)(void *io, char *s, size_t len);

typedef struct
{
    void *data;
    tty_iosop_t in;
    tty_iosop_t out;
} tty_ios_t;

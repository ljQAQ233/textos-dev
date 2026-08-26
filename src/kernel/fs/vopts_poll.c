#include <textos/fs.h>

int vfs_generic_poll_setup(node_t *this, struct fs_poller *poller)
{
    return 1;
}

int vfs_generic_poll_teardown(node_t *this, struct fs_poller *poller)
{
    return 1;
}

int vfs_poll_setup(node_t *this, struct fs_poller *poller)
{
    return this->opts->poll_setup(this, poller);
}

int vfs_poll_teardown(node_t *this, struct fs_poller *poller)
{
    return this->opts->poll_teardown(this, poller);
}

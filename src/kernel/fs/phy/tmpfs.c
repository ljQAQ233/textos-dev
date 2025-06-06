/*
 * /tmp is simple
 */

#include <textos/fs.h>

typedef struct tmpfs_super
{
    dev_t *dev;
} tmpfs_super_t;

/*
 * tmpfs initialization
 */
node_t *__fs_init_tmpfs()
{
}
#include <textos/net/socket.h>

// socket op

static sockop_t *ops[SOCK_T_MAX];

void sockop_set(int type, sockop_t *op)
{
    ops[type] = op;
}

sockop_t *sockop_get(int type)
{
    return ops[type];
}

// filesystem op

#include <textos/fs.h>
#include <textos/net/socket.h>

// iovec support
// NOTE: only one-member iovec is allowed now


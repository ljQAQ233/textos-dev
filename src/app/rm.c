#include <stdio.h>

#include "util.h"
PROG_NAME("rm");

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        pusage("Usage: %s file ...\n");
        return 1;
    }
    int err = 0;
    for (int i = 1; i < argc; i++) {
        char *file = argv[i];
        int ret = remove(file);
        if (ret < 0) error_x(file);
        err |= ret;
    }
    return !!err;
}

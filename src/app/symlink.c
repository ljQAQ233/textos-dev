#include <malloc.h>
#include <unistd.h>

#include "util.h"
PROG_NAME("symlink");

int main(int argc, char *argv[])
{
    if (argc != 3) {
        pusage("target linkto");
        return 1;
    }
    int ret = symlink(argv[1], argv[2]);
    if (ret < 0) error();
    return !!ret;
}

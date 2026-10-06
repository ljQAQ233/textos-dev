#include "../util.h"
#include <sys/stat.h>

PROG_NAME("mkfifo");

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (mkfifo(argv[i], 0666) < 0) {
            error();
        }
    }
    return 0;
}

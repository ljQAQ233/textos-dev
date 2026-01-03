#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
        return 1;
    int sec = atoi(argv[1]);
    sleep(sec);
    return 0;
}

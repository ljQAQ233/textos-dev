#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void __gentmp(char *sixx)
{
    static int init = 0;
    static unsigned int seed;
    if (!init) {
        pid_t pid = getpid();
        time_t ts = time(0);
        seed = (pid * 123567) ^ ts;
        init = 1;
    }

    rand_r(&seed);
    // do scale
    char suffix[12];
    unsigned rg0 = RAND_MAX;    // from
    unsigned rg2 = 1000000 - 0; // to
    unsigned num = 1ULL * seed * rg2 / rg0;
    snprintf(suffix, sizeof(suffix), "%06u\n", num);
    memcpy(sixx, suffix, 6);
}

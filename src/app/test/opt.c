#include <stdio.h>
#include <unistd.h>

// options:
//   -a arg / -b
// test with:
//   opt -b -aarg
//   opt -ba arg
//   opt -baarg
int main(int argc, char *argv[])
{
    char opt;
    while ((opt = getopt(argc, argv, ":a:b")) != -1) {
        switch (opt) {
        case 'a':
            printf("option a : arg=%s\n", optarg);
            break;
        case 'b':
            printf("option b\n");
            break;
        case ':':
            printf("option %c : arg=(missing)\n", optopt);
            break;
        }
    }
    printf("optind = %d\n", optind);
    return 0;
}

#include <stdio.h>

int opterr = 1; // if zero, getopt won't print error message to stderr
char *optarg;   // argument to the current option character
int optopt;     // option character that caused the error
int optind = 1; // index of the next element to be processed in argv
int __optoff = 0;
#define optoff __optoff

#define err(fmt, arg...) \
    if (opterr) fprintf(stderr, "%s: " fmt, argv[0], ##arg)

// POSIX.1-2008.
int getopt(int argc, char *argv[], const char *optstring)
{
    if (optind >= argc) return -1;
    if (argv[optind] == 0) return -1;
    // a new argv-element, scan it from the first character.
    // currently, string `--` can be used to terminate getopt operation.
    if (optoff == 0) {
        if (argv[optind][0] != '-') return -1;
        if (argv[optind][1] == '-' && argv[optind][2] == '\0') {
            return optind += 1, -1;
        }
        optoff++;
    }
    char cur = argv[optind][optoff];
    char opt = '?', arg = 0;
    for (int i = 0; optstring[i]; i++) {
        if (optstring[i] == cur) {
            opt = optstring[i];
            arg = optstring[i + 1];
        }
    }

    optoff++;
    if (opt == '?') {
        optopt = cur;
        err("invalid option -- %c\n", optopt);
        if (argv[optind][optoff] == '\0') {
            return optind++, optoff = 0, '?';
        }
    }
    if (arg == ':') {
        // take the remained part of this argv as an argument.
        // such as: `-aarg` is equivalent to `-a arg`
        if (argv[optind][optoff] != '\0') {
            optarg = argv[optind] + optoff;
            return optind += 1, optoff = 0, opt;
        }
        // otherwise, take the next argv element if it has
        if (optind + 1 >= argc || argv[optind + 1] == 0) {
            optopt = cur;
            if (argv[optind][optoff] == '\0') {
                optind += 1, optoff = 0;
            }
            if (optstring[0] == ':') return ':';
            err("option requires an argument -- %c\n", optopt);
            return '?';
        }
        optarg = argv[optind + 1];
        optind += 2, optoff = 0;
    } else if (argv[optind][optoff] == '\0') {
        // okay, handle the last case: if this option doesn't
        // take an argument, fix its optoff if needed
        optind += 1, optoff = 0;
    }
    return opt;
}

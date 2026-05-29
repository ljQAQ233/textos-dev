#include <unistd.h>
#include "unity/unity.h"

#define ARGC(x) (sizeof((x)) / sizeof((x)[0]) - 1)

static void reset(void)
{
    optind = 1, opterr = 0, optarg = 0;
}

void libc_getopt_basic(void)
{
    reset();

    char *argv[] = {"prog", "-a", "-b", NULL};
    int argc = ARGC(argv);

    TEST_ASSERT_EQUAL_INT('a', getopt(argc, argv, "ab"));
    TEST_ASSERT_EQUAL_INT(2, optind);
    TEST_ASSERT_EQUAL_INT('b', getopt(argc, argv, "ab"));
    TEST_ASSERT_EQUAL_INT(3, optind);

    // if called again, optind keeps the same
    TEST_ASSERT_EQUAL_INT(-1, getopt(argc, argv, "ab"));
    TEST_ASSERT_EQUAL_INT(3, optind);

    reset();
}

void libc_getopt_bundling(void)
{
    reset();

    char *argv[] = {"prog", "-ab", NULL};
    int argc = ARGC(argv);

    TEST_ASSERT_EQUAL_INT('a', getopt(argc, argv, "ab"));
    TEST_ASSERT_EQUAL_INT(1, optind);
    TEST_ASSERT_EQUAL_INT('b', getopt(argc, argv, "ab"));
    TEST_ASSERT_EQUAL_INT(2, optind);

    // if called again, optind keeps the same
    TEST_ASSERT_EQUAL_INT(-1, getopt(argc, argv, "ab"));
    TEST_ASSERT_EQUAL_INT(2, optind);

    reset();
}

void libc_getopt_with_arg(void)
{
    reset();

    char *argv[] = {"prog", "-b", "-a", "val", "-a", "-b", NULL};
    int argc = ARGC(argv);

    TEST_ASSERT_EQUAL_INT('b', getopt(argc, argv, "a:b"));
    TEST_ASSERT_EQUAL_INT('a', getopt(argc, argv, "a:b"));
    TEST_ASSERT_EQUAL_INT(4, optind);
    TEST_ASSERT_EQUAL_STRING("val", optarg);
    TEST_ASSERT_EQUAL_INT('a', getopt(argc, argv, "a:b"));
    TEST_ASSERT_EQUAL_STRING("-b", optarg);
    TEST_ASSERT_EQUAL_INT(-1, getopt(argc, argv, "a:b"));

    reset();
}

void libc_getopt_unknown(void)
{
    reset();

    char *argv[] = {"prog", "-x", NULL};
    int argc = ARGC(argv);

    TEST_ASSERT_EQUAL_INT('?', getopt(argc, argv, "a"));

    reset();
}

void libc_getopt_missing(void)
{
    reset();

    char *argv[] = {"prog", "-a", NULL};
    int argc = ARGC(argv);

    TEST_ASSERT_EQUAL_INT('?', getopt(argc, argv, "a:"));

    reset();
}

void libc_getopt_end(void)
{
    reset();

    char *argv[] = {"prog", "--", "-a", NULL};
    int argc = ARGC(argv);

    int c = getopt(argc, argv, "a");
    if (c == -1) {
        TEST_ASSERT_EQUAL_INT(2, optind);
    } else {
        TEST_ASSERT_EQUAL_INT('a', c);
    }

    reset();
}

void libc_getopt_adjacent(void)
{
    reset();

    char *argv[] = {"prog", "-abarg", "-b", NULL};
    int argc = ARGC(argv);

    TEST_ASSERT_EQUAL_INT('a', getopt(argc, argv, "a:b"));
    TEST_ASSERT_EQUAL_INT(2, optind);
    TEST_ASSERT_EQUAL_STRING("barg", optarg);
    TEST_ASSERT_EQUAL_INT('b', getopt(argc, argv, "a:b"));
    TEST_ASSERT_EQUAL_INT(3, optind);
    TEST_ASSERT_EQUAL_INT(-1, getopt(argc, argv, "a:b"));

    reset();
}

//! register=libc_getopt_basic
//! register=libc_getopt_bundling
//! register=libc_getopt_with_arg
//! register=libc_getopt_unknown
//! register=libc_getopt_missing
//! register=libc_getopt_end
//! register=libc_getopt_adjacent

#include "./unity/unity.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#define PREFIX "/tmp/libc_realpath"

static char buf[PATH_MAX];
static char prevdir[PATH_MAX];

static void prep()
{
    // we assume that /tmp is not a symlink
    if (mkdir(PREFIX, 0777) < 0) {
        if (errno == EEXIST) return;
        TEST_FAIL_MESSAGE("cannot create " PREFIX "\n");
    }
    mkdir(PREFIX "/dir", 0777);
    creat(PREFIX "/fil", 0777);
    symlink("../", PREFIX ".");
    symlink(PREFIX "////./.", PREFIX "/abs");
    symlink("/", PREFIX "/dir/root");
}

static void pushd()
{
    if (getcwd(prevdir, sizeof(prevdir)) < 0)
        TEST_FAIL_MESSAGE("cannot getcwd()\n");
    if (chdir(PREFIX) < 0)
        TEST_FAIL_MESSAGE("cannot chdir(\"" PREFIX "\")\n");
}

static void popd()
{
    if (chdir(prevdir) < 0)
        TEST_FAIL_MESSAGE("cannot chdir(prevdir)\n");
}

void libc_realpath_common()
{
    prep();
    pushd();

    TEST_ASSERT_NOT_NULL(realpath(PREFIX "/abs", buf));
    TEST_ASSERT_EQUAL_STRING(PREFIX, buf);

    TEST_ASSERT_NOT_NULL(realpath(PREFIX "/dir", buf));
    TEST_ASSERT_EQUAL_STRING(PREFIX "/dir", buf);

    TEST_ASSERT_NOT_NULL(realpath(PREFIX "/dir/root", buf));
    TEST_ASSERT_EQUAL_STRING("/", buf);

    TEST_ASSERT_NOT_NULL(realpath(PREFIX "/fil", buf));
    TEST_ASSERT_EQUAL_STRING(PREFIX "/fil", buf);

    TEST_ASSERT_NULL(realpath(PREFIX "/fil/something_not_dir", buf));
    TEST_ASSERT_EQUAL_INT(ENOTDIR, errno);

    popd();
}

void libc_realpath_dots()
{
    prep();
    pushd();

    TEST_ASSERT_NOT_NULL(realpath(PREFIX "/dir/././.././", buf));
    TEST_ASSERT_EQUAL_STRING(PREFIX, buf);

    TEST_ASSERT_NOT_NULL(realpath(PREFIX "/dir/root/..", buf));
    TEST_ASSERT_EQUAL_STRING("/", buf);
    
    TEST_ASSERT_NULL(realpath(PREFIX "/./fil/../", buf));
    TEST_ASSERT_NULL(realpath(PREFIX "/./fil/./", buf));

    popd();
}

void libc_realpath_complex()
{
    prep();
    pushd();

    TEST_ASSERT_NOT_NULL(realpath(PREFIX "/////////dir/////././.././///", buf));
    TEST_ASSERT_EQUAL_STRING(PREFIX, buf);

    TEST_ASSERT_NOT_NULL(
        realpath(PREFIX "/dir/root/..////////" PREFIX "/abs", buf));
    TEST_ASSERT_EQUAL_STRING(PREFIX, buf);

    popd();
}

//! register=libc_realpath_common
//! register=libc_realpath_dots
//! register=libc_realpath_complex

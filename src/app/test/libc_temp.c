#include "unity/unity.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void libc_mkostemp()
{
    char buf[] = //
        "Hello World\n"
        "Some test strings\n";
    char echo[sizeof(buf)];
    char template[] = "/tmp/testXXXXXX.log";
    int fd = mkostemps(template, 4, 0);
    TEST_ASSERT_GREATER_OR_EQUAL(0, fd);
    TEST_ASSERT(strcmp(template, "/tmp/testXXXXXX.log") != 0);
    write(fd, buf, sizeof(buf));
    lseek(fd, SEEK_SET, 0);
    read(fd, echo, sizeof(buf));
    close(fd);
    remove(template);
    TEST_ASSERT(memcmp(buf, echo, sizeof(buf)) == 0);
}

//! register=libc_mkostemp

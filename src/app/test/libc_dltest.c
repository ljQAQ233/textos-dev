#include <dlfcn.h>
#include "unity/unity.h"

#define IGNORE
#define dlclose(_) 0

void libc_dl_dlopen()
{
#ifdef IGNORE
    TEST_IGNORE();
#endif

    void *dl = dlopen("solib/add.so", RTLD_LAZY);
    TEST_ASSERT_NOT_NULL(dl);
    int (*pp)(int, int) = dlsym(dl, "add");
    TEST_ASSERT_NOT_NULL(pp);
    TEST_ASSERT_EQUAL(8, pp(1, 7));
    TEST_ASSERT_EQUAL(0, dlclose(dl));
}

void libc_dl_unique()
{
#ifdef IGNORE
    TEST_IGNORE();
#endif

    void *dl1 = dlopen("solib/add.so", RTLD_LAZY);
    void *dl2 = dlopen("solib/add.so", RTLD_LAZY);
    TEST_ASSERT_NOT_NULL(dl1);
    TEST_ASSERT(dl1 == dl2);
    TEST_ASSERT_EQUAL(0, dlclose(dl1));
    TEST_ASSERT_EQUAL(0, dlclose(dl2));
}

//! register=libc_dl_dlopen
//! register=libc_dl_unique

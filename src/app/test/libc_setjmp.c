#include <setjmp.h>
#include "unity/unity.h"

static jmp_buf a, b;
static void A();
static void B();

static void A()
{
    int r;
    r = setjmp(a);
    if (r == 0) B();

    TEST_ASSERT_EQUAL(10001, r);
    r = setjmp(a);
    if (r == 0) longjmp(b, 20001);

    TEST_ASSERT_EQUAL(10002, r);
    r = setjmp(a);
    if (r == 0) longjmp(b, 20002);

    TEST_ASSERT_EQUAL(10003, r);
}

static void B()
{
    int r;
    r = setjmp(b);
    if (r == 0) longjmp(a, 10001);

    TEST_ASSERT_EQUAL(20001, r);
    r = setjmp(b);
    if (r == 0) longjmp(a, 10002);

    TEST_ASSERT_EQUAL(20002, r);
    r = setjmp(b);
    if (r == 0) longjmp(a, 10003);
}

void libc_setjmp()
{
    A();
}
//!register=libc_setjmp

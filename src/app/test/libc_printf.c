#include "unity/unity.h"
#include <stdio.h>

void libc_printf_f_basic(void)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "%f", 0.0);
    TEST_ASSERT_EQUAL_STRING("0.000000", buf);

    snprintf(buf, sizeof(buf), "%f", 1.0);
    TEST_ASSERT_EQUAL_STRING("1.000000", buf);

    snprintf(buf, sizeof(buf), "%f", 42.0);
    TEST_ASSERT_EQUAL_STRING("42.000000", buf);
}

void libc_printf_f_negative(void)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "%f", -1.0);
    TEST_ASSERT_EQUAL_STRING("-1.000000", buf);

    snprintf(buf, sizeof(buf), "%f", -3.14);
    TEST_ASSERT_EQUAL_STRING("-3.140000", buf);

    snprintf(buf, sizeof(buf), "%f", -0.0);
    TEST_ASSERT_EQUAL_STRING("-0.000000", buf);
}

void libc_printf_f_fraction(void)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "%f", 0.5);
    TEST_ASSERT_EQUAL_STRING("0.500000", buf);

    snprintf(buf, sizeof(buf), "%f", 0.25);
    TEST_ASSERT_EQUAL_STRING("0.250000", buf);

    snprintf(buf, sizeof(buf), "%f", 0.125);
    TEST_ASSERT_EQUAL_STRING("0.125000", buf);

    snprintf(buf, sizeof(buf), "%f", 0.0625);
    TEST_ASSERT_EQUAL_STRING("0.062500", buf);

    snprintf(buf, sizeof(buf), "%f", 1.5);
    TEST_ASSERT_EQUAL_STRING("1.500000", buf);

    snprintf(buf, sizeof(buf), "%f", 2.75);
    TEST_ASSERT_EQUAL_STRING("2.750000", buf);
}

void libc_printf_f_precision(void)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "%.2f", 3.141592653589793);
    TEST_ASSERT_EQUAL_STRING("3.14", buf);

    snprintf(buf, sizeof(buf), "%.3f", 3.141592653589793);
    TEST_ASSERT_EQUAL_STRING("3.142", buf);

    snprintf(buf, sizeof(buf), "%.0f", 3.141592653589793);
    TEST_ASSERT_EQUAL_STRING("3", buf);

    snprintf(buf, sizeof(buf), "%.1f", 0.5);
    TEST_ASSERT_EQUAL_STRING("0.5", buf);

    snprintf(buf, sizeof(buf), "%.5f", 1.0);
    TEST_ASSERT_EQUAL_STRING("1.00000", buf);

    snprintf(buf, sizeof(buf), "%.10f", 0.0);
    TEST_ASSERT_EQUAL_STRING("0.0000000000", buf);
}

void libc_printf_f_width(void)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "%10f", 1.0);
    TEST_ASSERT_EQUAL_STRING("  1.000000", buf);

    snprintf(buf, sizeof(buf), "%-10f", 1.0);
    TEST_ASSERT_EQUAL_STRING("1.000000  ", buf);

    snprintf(buf, sizeof(buf), "%010f", 1.0);
    TEST_ASSERT_EQUAL_STRING("001.000000", buf);

    snprintf(buf, sizeof(buf), "%+f", 1.0);
    TEST_ASSERT_EQUAL_STRING("+1.000000", buf);

    snprintf(buf, sizeof(buf), "% f", 1.0);
    TEST_ASSERT_EQUAL_STRING(" 1.000000", buf);

    snprintf(buf, sizeof(buf), "%+f", -1.0);
    TEST_ASSERT_EQUAL_STRING("-1.000000", buf);
}

void libc_printf_f_return(void)
{
    int ret;
    char buf[128];

    ret = snprintf(buf, sizeof(buf), "%f", 1.0);
    TEST_ASSERT_GREATER_OR_EQUAL(0, ret);

    ret = snprintf(buf, sizeof(buf), "%.2f", 3.14);
    TEST_ASSERT_GREATER_OR_EQUAL(4, ret);
}

void libc_printf_f_large(void)
{
    char buf[128];

    snprintf(buf, sizeof(buf), "%f", 1e10);
    TEST_ASSERT_EQUAL_STRING("10000000000.000000", buf);

    snprintf(buf, sizeof(buf), "%f", 1e-6);
    TEST_ASSERT_EQUAL_STRING("0.000001", buf);
}

//! register=libc_printf_f_basic
//! register=libc_printf_f_negative
//! register=libc_printf_f_fraction
//! register=libc_printf_f_precision
//! register=libc_printf_f_width
//! register=libc_printf_f_return
//! register=libc_printf_f_large

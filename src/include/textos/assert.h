#ifndef __ASSERT_H__
#define __ASSERT_H__

void assertk (
        const char *file,
        const u64  line,
        const bool state,
        const char *expr
        );

#define ASSERTK(expr) \
            assertk (__FILE__, __LINE__, (expr), #expr)

#endif

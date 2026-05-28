#include "unity/unity.h"
#include "auto/unity.h"

#define _(file, func, name, line) extern void func();
TESTS
#undef _

struct unity_params
{
    const char *file;
    UnityTestFunction func;
    const char *name;
    UNITY_LINE_TYPE line;
} unity_params[] = {
#define _(file, func, name, line) {file, func, name, line},
    TESTS
#undef _
};

void setUp(void)
{}
void tearDown(void)
{}
void verifyTest(void);
void verifyTest(void)
{}

void resetTest(void);
void resetTest(void)
{
    tearDown();
    setUp();
}

static void runtest(struct unity_params *param)
{
    UnitySetTestFile(param->file);
    Unity.CurrentTestName = param->name;
    Unity.CurrentTestLineNumber = (UNITY_UINT)param->line;
#ifdef UNITY_USE_COMMAND_LINE_ARGS
    if (!UnityTestMatches()) return;
#endif
    Unity.NumberOfTests++;
    UNITY_CLR_DETAILS();
    UNITY_EXEC_TIME_START();
    if (TEST_PROTECT()) {
        setUp();
        param->func();
    }
    if (TEST_PROTECT()) {
        tearDown();
    }
    UNITY_EXEC_TIME_STOP();
    UnityConcludeTest();
}

int main(int argc, char **argv)
{
#ifdef UNITY_USE_COMMAND_LINE_ARGS
    int parse_status = UnityParseOptions(argc, argv);
    if (parse_status != 0) {
        if (parse_status < 0) {
            UnityPrint("setjmp.");
            UNITY_PRINT_EOL();
            return 0;
        }
        return parse_status;
    }
#endif
    UnityBegin("test");

    int num = sizeof(unity_params) / sizeof(unity_params[0]);
    for (int i = 0; i < num; i++) {
        runtest(&unity_params[i]);
    }

    return UNITY_END();
}

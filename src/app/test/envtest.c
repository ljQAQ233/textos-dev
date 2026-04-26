#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
    
extern char **environ;

#define NRAND 10

#define envcmp(a, b) strcmp(getenv(a), b)
#define chk(x)      \
    if (!(x)) {     \
        dump_env(); \
        assert(0);  \
    }

void dump_env()
{
    puts("====== dump env ======");
    char **env = environ;
    while (*env)
        puts(*env++);
    puts("====== dump end ======");
}

void normal_tests()
{
    chk(!setenv("PATH", "/bin", 1));
    chk(!envcmp("PATH", "/bin"));
    
    chk(!setenv("PATH", "/sbin", 0));
    chk(!envcmp("PATH", "/bin"));

    chk(!setenv("PATH", "/sbin", 1));
    chk(!envcmp("PATH", "/sbin"));

    chk(!unsetenv("PATH"));
    chk(!getenv("PATH"));
}

char *vars[] = {
    "A_VERY_LOOONG_VARNAME", "A",    "B", "D", "E", "F", "G",
    "WHAT_THE_DOG_DOING",    "KAWA", "J",
};

int rand();
void srand(unsigned);

void random_tests()
{
    char buf[1000];
    int nr = sizeof(vars) / sizeof(vars[0]);
    long long val[nr];
    memset(val, 0x7f, sizeof(val));
    srand(time(0));
    for (int i = 0 ; i < NRAND ; i++) {
        int idx = rand() % nr;
        char *var = vars[idx];
        long long mess = 1LL * rand() * rand() * rand();
        snprintf(buf, sizeof(buf), "%lld", mess);
        chk(!setenv(var, buf, 1));
        val[idx] = mess;
        printf("set %s = %s\n", var, buf);
    }
    for (int i = 0 ; i < nr ; i++) {
        if (val[i] == 0x7f7f7f7f7f7f7f7fll) continue;
        chk(getenv(vars[i]) != 0);
        chk(atoll(getenv(vars[i])) == val[i]);
    }
}

int main(int argc, char *argv[])
{
    normal_tests();
    random_tests();

    printf("all test cases passed\n");
    return 0;
}

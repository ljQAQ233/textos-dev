#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    assert(!setenv("PATH", "/bin", 0));
    printf("1. PATH = %s\n", getenv("PATH"));
    
    assert(!setenv("PATH", "/sbin", 0));
    printf("2. PATH = %s\n", getenv("PATH"));

    assert(!setenv("PATH", "/sbin", 1));
    printf("3. PATH = %s\n", getenv("PATH"));

    assert(!unsetenv("PATH"));
    printf("4. PATH = %s\n", getenv("PATH"));
    return 0;
}
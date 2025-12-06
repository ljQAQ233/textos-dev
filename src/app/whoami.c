#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    pid_t euid = geteuid();
    struct passwd *pwd = getpwuid(euid);
    if (pwd == NULL) {
        perror(NULL);
        return 1;
    }
    printf("%s\n", pwd->pw_name);
    return 0;
}

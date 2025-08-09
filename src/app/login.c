#include <pwd.h>
#include <stdio.h>
#include <crypt.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>

#define TRYMAX 3
#define BUFMAX 128

char name[BUFMAX];
char user[BUFMAX];
char pass[BUFMAX];

void set_echo(int yes)
{
    struct termios tio;
    tcgetattr(0, &tio);
    if (yes)
        tio.c_lflag |= ECHO;
    else
        tio.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &tio);
}

struct passwd *login()
{
    printf("%s login: ", name);

    fgets(user, sizeof(user), stdin);
    user[strlen(user) - 1] = '\0';
    set_echo(0);

    struct passwd *pwd = getpwnam(user);
    for (int try = 0 ; try < TRYMAX ; try++)
    {
        printf("Password: ");
        fgets(pass, sizeof(pass), stdin);
        if (!pwd)
            goto retry;
        char *cry = crypt(pass, pwd->pw_passwd);
        if (!strcmp(cry, pwd->pw_passwd))
        {
            set_echo(1);
            return pwd;
        }
    retry:
        printf("\nLogin incorrect\n");
    }
    set_echo(1);
    return NULL;
}

char *argv[2];
extern char **__environ;

int main()
{
    if (gethostname(name, sizeof(name)) < 0)
        strcpy(name, "localhost");

    setsid();
    struct passwd *pwd;
    while (!(pwd = login()));

    setenv("USER", pwd->pw_name, 1);
    setenv("HOME", pwd->pw_dir, 1);
    setenv("SHELL", pwd->pw_shell, 1);
    setenv("PATH", "/bin", 1);

    setgid(pwd->pw_uid);
    setuid(pwd->pw_uid);
    chdir(pwd->pw_dir);

    argv[0] = pwd->pw_shell;
    argv[1] = NULL;
    execve(argv[0], argv, __environ);
    return 1;
}

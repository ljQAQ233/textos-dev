#include <pwd.h>
#include "../stdio/stdio.h"

// 内部函数返回值
#define PW_OK    1
#define PW_NOENT 0
#define PW_ERR  -1

int __pwdup(struct passwd *pwd, char *buf, size_t len);
int __getpwent(struct passwd *pwd, char **ln, size_t *sz, FILE *fp);
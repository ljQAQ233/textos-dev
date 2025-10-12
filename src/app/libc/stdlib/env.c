#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>

char **__environ;
__weak_alias(__environ, environ);

/**
 * @brief 记录指针的内存使用情况, 是固定的还是 malloc 分配的?
 * 
 * @param oe 原来的指针
 * @param ne 新的指针
 * @return void* 返回新指针
 */
static void *__addenv(void *oe, void *ne)
{
    size_t i;
    static void **env_alloc;
    static size_t env_asize;
    for (i = 0 ; i < env_asize ; i++)
    {
        if (env_alloc[i] == oe) {
            if (oe)
                free(oe);
            return env_alloc[i] = ne;
        }
    }
    
    for (i = 0 ; i < env_asize ; i++)
        if (env_alloc[i] == NULL)
            return env_alloc[i] = ne;

    void **env_alloc_new = malloc(sizeof(void *) * (env_asize + 1));
    for (i = 0 ; i < env_asize ; i++)
        env_alloc_new[i] = env_alloc[i];
    free(env_alloc);
    env_alloc = env_alloc_new;
    return env_alloc[env_asize++] = ne;
}

/**
 * @brief 将 s 添加到环境变量中
 * 
 * @param s 新的环境变量
 * @param l 环境变量中名字的长度
 */
static int __putenv(char *s, size_t l)
{
    size_t i;
    char **e = (char **)__environ;
    for (i = 0 ; e[i] ; i++) {
        if (strncmp(e[i], s, l))
            continue;
        e[i] = s;
        __addenv(e[i], s);
        return 0;
    }

    // clone
    char **en = malloc((i + 2) * sizeof(char *));
    for (i = 0 ; e[i] ; i++)
        en[i] = e[i];
    en[i++] = s;
    en[i] = NULL;
    __environ = __addenv(e, en);
    return 0;
}

int setenv(const char *name, const char *value, int overwrite)
{
    size_t l1 = strchrnul(name, '=') - name;
    if (!name || !l1 || name[l1]) {
        errno = EINVAL;
        return -1;
    }
    if (!overwrite && getenv(name))
        return 0;

    size_t l2 = strlen(value);
    char *s = malloc(l1+l2+2);
    if (!s)
        return -1;
    s[l1] = '=';
    memcpy(s, name, l1);
    memcpy(s+l1+1, value, l2+1);
    return __putenv(s, l1);
}

int putenv(char *str)
{
    size_t l = strchrnul(str, '=') - str;
	if (!l || !str[l])
        return unsetenv(str);
	return __putenv(str, l);
}

int unsetenv(const char *name)
{
    size_t l1 = strchrnul(name, '=') - name;
    if (!l1 || name[l1]) {
        errno = EINVAL;
        return -1;
    }

    if (__environ)
    {
        size_t i = 0;
        char **e = (char **)__environ;
        for ( ; e[i] ; i++) {
            if (!strncmp(e[i], name, l1)) {
                e[i] = __addenv(e[i], NULL);
                break;
            }
        }
        for ( ; e[i+1] ; i++)
            e[i] = e[i+1];
    }
    return 0;
}

char *getenv(const char *name)
{
    char **e = __environ;
    size_t len = strchrnul(name, '=') - name;
    if (len && !name[len] && e) {
        for (int i = 0 ; e[i] ; i++) {
            if (strncmp(e[i], name, len))
                continue;
            if (e[i][len] != '=')
                continue;
            return (char *)&e[i][len + 1];
        }
    }
    return NULL;
}

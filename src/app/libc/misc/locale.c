#include <limits.h>
#include <locale.h>
#include <string.h>

static struct lconv c_locale = {
    ".",      "",       "",       "",       "",       "",
    "",       "",       "",       "",       CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX};

static const char *current_locale = "C";

char *setlocale(int category, const char *locale)
{
    if (locale == NULL) {
        return (char *)current_locale;
    }

    if (locale[0] == '\0' || strcmp(locale, "C") == 0 ||
        strcmp(locale, "POSIX") == 0) {
        return (char *)locale;
    }

    if (locale[0] == '\0') {
        return "C";
    }

    return NULL;
}

struct lconv *localeconv()
{
    return &c_locale;
}

int strcoll(const char *s1, const char *s2)
{
    return strcmp(s1, s2);
}

size_t strxfrm(char *dest, const char *src, size_t n)
{
    size_t len = 0;
    while (src[len] != '\0')
        len++;

    if (n > 0) {
        size_t i;
        for (i = 0; i < n - 1 && src[i] != '\0'; i++)
            dest[i] = src[i];
        dest[i] = '\0';
    }
    return len;
}

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

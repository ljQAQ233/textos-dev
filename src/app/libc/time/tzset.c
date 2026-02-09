#include "time.h"
#include <string.h>
#include <stdlib.h>

static struct entry
{
    const char *tzname;
    time_t tmzone;
} db[] = {
    {"UTC", 0},
    {"GMT", 0},
    {0, 0},
};

char *tzname[2] = {"UTC", "UTC"};
long timezone = +0;
int daylight = 0;

void tzset()
{
    char *tz = getenv("TZ");
    struct entry *r = NULL;
    if (tz) {
        for (struct entry *e = db; e->tzname; e++) {
            int l = strlen(e->tzname);
            if (strncmp(tz, e->tzname, l) == 0) {
                r = e;
                tz += l;
                break;
            }
        }
    }
    if (!r) return;
    int off = strtol(tz, &tz, 10);
    timezone = r->tmzone + off * TS_HOUR;
    tzname[0] = tzname[1] = (char *)r->tzname;
    daylight = 0;
}

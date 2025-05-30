#pragma once

#include <textos/time.h>

u64 time_stamp (rtc_tm_t *tm);

void time_rtctm(u64 ts, rtc_tm_t *tm);


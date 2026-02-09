import time
import random
import calendar

# Assume that the gmtime in libc supportes the most of time stamps in this range.
# The expected proportion of invalid cases is approximately 11% to 13%.
OFFSET = 2**53
N = 5000

precases = [
    time.strptime(
        "Jan 1 12:00:00 AM UTC 1967",
        "%b %d %I:%M:%S %p %Z %Y"),
    time.strptime(
        "Jan 9 11:45:14 AM UTC 0001",
        "%b %d %I:%M:%S %p %Z %Y"),
]

INT_MIN = -1 - 0x7fffffff
INT_MAX = 0x7fffffff
TS_LEAP_Y = 31622400
minv = INT_MIN * TS_LEAP_Y - OFFSET
maxv = INT_MAX * TS_LEAP_Y + OFFSET

def fmt(stamp, valid, tm: time.struct_time):
    # leap second is not allowed
    assert tm is None or tm.tm_sec < 60
    if not valid:
        return f"{{{stamp}, 0, {{ 0 }} }},"
    wday = (tm.tm_wday + 1) % 7
    return (
        f"{{{stamp}, 1, "
        f"{{ {tm.tm_sec}, {tm.tm_min}, {tm.tm_hour}, "
        f"{tm.tm_mday}, {tm.tm_mon - 1}, {tm.tm_year - 1900}, "
        f"{wday}, {tm.tm_yday - 1} }} }},"
    )

def gen_cases():
    for tm in precases:
        yield calendar.timegm(tm), True, tm

    for _ in range(N):
        stamp = random.randrange(minv, maxv)
        try:
            tm = time.gmtime(stamp)
            yield stamp, True, tm
        except (ValueError, OSError):
            yield stamp, False, None

def main():
    cases = list(gen_cases())
    inv = sum(not v for _, v, _ in cases)

    for c in cases:
        print(fmt(*c))

    print(f"// random [{minv:e}, {maxv:e})")
    print(f"// number of cases: {len(cases)}")
    print(f"// P(invalid cases) = {inv/len(cases):.8f}")


if __name__ == "__main__":
    main()

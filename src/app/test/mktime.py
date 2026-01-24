import time
import random

N = 2000

for _ in range(N):
    stamp = random.getrandbits(32)
    tm = time.gmtime(stamp)
    print(
        f"{{{stamp}, "
        f"{{ {tm.tm_sec}, {tm.tm_min}, {tm.tm_hour}, "
        f"{tm.tm_mday}, {tm.tm_mon - 1}, {tm.tm_year - 1900}, "
        f"{tm.tm_wday}, {tm.tm_yday} }} }},"
    )

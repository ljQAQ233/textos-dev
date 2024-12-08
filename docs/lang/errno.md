# musl

- `perror` / `strerr`

提取按 `errno.h` 排了序之后的 `errstr`

```sh
E="errno.h"
S="__strerror.h"

cat $E | while read -r line
do
    grep "$(echo ${line} | awk '{print $2}')" $S 
done
```
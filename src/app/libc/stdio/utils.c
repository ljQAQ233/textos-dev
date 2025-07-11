#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int __fmode(const char *mode)
{
    if (!mode)
        return -EINVAL;
    int rw = 0;
    int flg = 0;
    while (*mode)
    {
        switch (*mode++)
        {
        case 'r':
            rw |= 1;
            break;
        case 'w':
            rw |= 2;
            break;
        case '+':
            rw |= 3;
            flg |= O_CREAT;
            break;
        case 'a':
            rw |= 2;
            flg |= O_CREAT | O_APPEND;
            break;
        default:
            return -EINVAL;
        }
    }
    if (rw == 1)
        flg |= O_RDONLY;
    else if (rw == 2)
        flg |= O_WRONLY;
    else if (rw == 3)
        flg |= O_RDWR;
    else
        return -EINVAL;
    return flg;
}

static FILE *ofl_head;

void __ofl_add(FILE *f)
{
    f->_f_next = ofl_head;
    ofl_head = f;
}

void __ofl_del(FILE *f)
{
    FILE **pp = &ofl_head;
    while (*pp && *pp != f)
        pp = (FILE **)&((*pp)->_f_next);
    if (*pp == f)
        *pp = (*pp)->_f_next;
}
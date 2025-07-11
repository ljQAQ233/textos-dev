#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include "stdio.h"

int fclose(FILE *stream)
{
    __ofl_del(stream);
    close(stream->_f_fd);
    free(stream);
    return 0;
}
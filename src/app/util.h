#pragma once

extern char _prog_name[];

#define PROG_NAME(name) char _prog_name[] = name

#include <stdio.h>

static void error()
{
    fprintf(stderr, "%s: %m\n", _prog_name);
}

static void error_x(const char *x)
{
    fprintf(stderr, "%s: %s: %m\n", _prog_name, x);
}

static void pusage(const char *usage)
{
    fprintf(stderr, "Usage: %s %s\n", _prog_name, usage);
}

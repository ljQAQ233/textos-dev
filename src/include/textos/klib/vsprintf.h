#pragma once

#include <textos/args.h>

int vsprintf(char *buffer, const char *format, va_list args);

int sprintf(char *buffer, const char *format, ...);

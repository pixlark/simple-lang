#pragma once
#include "common.h"

void fatal(const char * fmt, ...);
void _internal_error(const char * fmt, char * file, int line, ...);

#define internal_error(fmt, ...) _internal_error(fmt, __FILE__, __LINE__, ##__VA_ARGS__)

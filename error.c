#include "error.h"

void fatal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("Fatal Error:\n  ");
	vprintf(fmt, args);
	printf("\n");

	va_end(args);
	exit(1);
}

void fatal_line(u32 line, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("Fatal Error:\n  [:%d] ", line);
	vprintf(fmt, args);
	printf("\n");

	va_end(args);
	exit(1);	
}

void runtime(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("Runtime Error:\n  ");
	vprintf(fmt, args);
	printf("\n");

	va_end(args);
	exit(1);
}

void runtime_line(u32 line, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("Runtime Error:\n  [:%d] ", line);
	vprintf(fmt, args);
	printf("\n");

	va_end(args);
	exit(1);	
}

void _internal_error(const char * fmt, char * file, int line, ...)
{
	va_list args;
	va_start(args, line);

	printf("Internal Compiler Error:\nFile: %s | Line: %d\n  ", file, line);
	vprintf(fmt, args);
	printf("\n");

	va_end(args);
	exit(1);
}

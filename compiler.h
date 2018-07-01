#pragma once

#include "common.h"
//#include "vm.h"
#include "parser.h"

typedef struct Declaration {
	const char * name;
	size_t size;
} Declaration;

#if 0
void compile_expression(VM * vm, Expression * expression);
void compile_statement(VM * vm, Statement * stmt);
#endif

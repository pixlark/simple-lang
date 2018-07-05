#pragma once

#include "common.h"
#include "parser.h"

// From parser.h
typedef struct Function Function;
//

typedef struct Declaration {
	const char * name;
	size_t size;
	int decl_pos;
} Declaration;

Declaration * read_function_decls(Function * func);
void compile_function(Function * func);

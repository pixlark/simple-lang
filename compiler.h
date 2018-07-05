#pragma once

#include "common.h"
#include "parser.h"
#include "vm.h"

// From parser.h
typedef struct Expression Expression;
typedef struct Statement Statement;
typedef struct Function Function;
//

// From vm.h
typedef struct VM VM;
//

typedef struct Declaration {
	const char * name;
	size_t size;
	int decl_pos;
} Declaration;

void prepare_function(Function * func);
void compile_function(VM * vm, Function * func);

void compile_expression(VM * vm, Expression * expr);
void compile_statement(VM * vm, Statement * stmt);

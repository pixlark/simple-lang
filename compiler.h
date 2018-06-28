#pragma once

#include "common.h"
#include "vm.h"
#include "parser.h"

void compile_expression(VM * vm, Expression * expression);
void compile_statement(VM * vm, Statement * stmt);

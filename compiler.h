#pragma once

#include "common.h"
#include "vm.h"
#include "parser.h"

#define COMPILE_TEST_DEBUG  false
#define CPU_STATE_REPORTING false

void compile_expression(VM * vm, Expression * expression);
void compile_test();

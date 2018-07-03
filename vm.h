#pragma once

#include "common.h"

#define STACK_SIZE 1024

typedef enum Inst_Type {
	INST_NOP,
	INST_PUSH_OP,
	INST_PUSH_CALL,
} Inst_Type;

typedef union Inst_Arg {
	u64 literal;
} Inst_Arg;

typedef struct Inst {
	Inst_Type type;
	Inst_Arg  arg0;
	Inst_Arg  arg1;
} Inst;

typedef struct VM {
	u64 op_stack[STACK_SIZE];
	u64 op_sp;
	u64 call_stack[STACK_SIZE];
	u64 call_sp;

	Inst * insts;
} VM;

int vm_init(VM * vm);
void vm_test();

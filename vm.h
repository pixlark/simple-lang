#pragma once

#include "common.h"
#include "error.h"
#include "parser.h"

#define STACK_SIZE 1024

typedef enum Inst_Type {
	INST_HALT,
	INST_NOP,
	INST_OP,
	// Call stack
	INST_PUSHC, // Push literal onto call stack
	INST_POPC,  // Pop the top of call stack
	// Op stack
	INST_PUSHO, // Push literal onto op stack
	INST_POPO,  // Pop the top of op stack
	// Inter-stack movement
	INST_LOAD,  // Load from offset into call stack onto op stack
	INST_SAVE,  // Pop top of op stack and save into offset into call stack
} Inst_Type;

extern char * inst_type_to_str[];

typedef union Inst_Arg {
	u64 literal;
	u64 offset;
	Operator_Type op_type;
} Inst_Arg;

typedef struct Inst {
	Inst_Type type;
	Inst_Arg  arg0;
} Inst;

typedef struct VM {
	s64 op_stack[STACK_SIZE];
	u64 op_sp;
	
	s64 call_stack[STACK_SIZE];
	u64 call_sp;
	
	Inst * insts;
	u64 ip;
} VM;

int vm_init(VM * vm);
void vm_test();

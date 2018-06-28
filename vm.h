#pragma once
#include "common.h"
#include "parser.h"
#include "map.h"

typedef enum Item_Type {
	ITEM_VARIABLE,
	ITEM_LITERAL,
} Item_Type;

typedef struct Item {
	Item_Type type;
	union {
		struct {
			char * name;
		} variable;
		struct {
			s64 val;
		} literal;
	};
} Item;

typedef struct Stack {
	Item * arr;
} Stack;

typedef enum Inst_Type {
	INST_HALT,
	INST_OPERATOR,
	INST_RESOLVE,
	INST_PUSH,
} Inst_Type;

typedef struct Instruction {
	Inst_Type type;
	union {
		Operator_Type op_type;
		Item item;
		char * name;
	} arg0;
} Instruction;

typedef struct VM {
	Map * symbol_table;
	Stack stack;
	int pc;
	Instruction * instructions;
} VM;

extern void (*operators[])(VM*);

void vm_test();

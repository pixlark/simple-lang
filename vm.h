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
			const char * name;
		} variable;
		struct {
			s64 val;
		} literal;
	};
} Item;

void print_item(Item item);

typedef struct Stack {
	Item * arr;
} Stack;

void stack_init(Stack * stack);
Item stack_at(Stack * stack, int i);
void stack_push(Stack * stack, Item item);
Item stack_pop(Stack * stack);

typedef enum Inst_Type {
	INST_HALT,
	INST_PRINT,
	INST_OPERATOR,
	INST_RESOLVE,
	INST_BIND,
	INST_PUSH,
	INST_POP,
	INST_JZ,
	INST_JNZ,
	INST_JUMP,
} Inst_Type;

typedef struct Instruction {
	Inst_Type type;
	union {
		Item item;
		Operator_Type op_type;
		const char * name;
		u32 jmp_mark;
	} arg0;
} Instruction;

void print_instruction(Instruction inst);

#define sat(x)   (stack_at(&vm->stack, (x)))
#define spop()   (stack_pop(&vm->stack))
#define spush(x) (stack_push(&vm->stack, (x)))

#define ipush(x) (sb_push(vm->instructions, (x)))

typedef struct VM {
	Map * symbol_table;
	Stack stack;
	int pc;
	Instruction * instructions;
} VM;

void vm_init(VM * vm);
bool vm_step(VM * vm);
void vm_print_state(VM * vm);

extern void (*operators[])(VM*);

void vm_test();

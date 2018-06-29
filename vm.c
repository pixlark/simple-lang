#include "vm.h"

/*
 * Items
 */

void print_item(Item item)
{
	switch (item.type) {
	case ITEM_VARIABLE:
		printf("Variable '%s'\n", item.variable.name);
		break;
	case ITEM_LITERAL:
		printf("Literal %ld\n", item.literal.val);
		break;
	}
}

Item make_item_variable(char * name)
{
	Item item;
	item.type = ITEM_VARIABLE;
	item.variable.name = name;
	return item;
}

Item make_item_literal(s64 val)
{
	Item item;
	item.type = ITEM_LITERAL;
	item.literal.val = val;
	return item;
}

/*
 * Stack
 */

void stack_init(Stack * stack)
{
	stack->arr = 0;
}

Item stack_at(Stack * stack, int i)
{
	return stack->arr[sb_count(stack->arr) - i - 1];
}

void stack_push(Stack * stack, Item item)
{
	sb_push(stack->arr, item);
}

Item stack_pop(Stack * stack)
{
	return sb_pop(stack->arr);
}

/*
 * Instruction
 */

char * inst_to_str[] = {
	[INST_HALT]     = "HALT",
	[INST_PRINT]    = "PRINT",
	[INST_OPERATOR] = "OPERATOR",
	[INST_RESOLVE]  = "RESOLVE",
	[INST_BIND]     = "BIND",
	[INST_PUSH]     = "PUSH",
	[INST_POP]      = "POP",
	[INST_JZ]       = "JZ",
	[INST_JNZ]      = "JNZ",
	[INST_JUMP]     = "JUMP"
};

void print_instruction(Instruction inst)
{
	printf("%s ", inst_to_str[inst.type]);
	switch (inst.type) {
	case INST_OPERATOR:
		printf("%s\n", op_to_str[inst.arg0.op_type]);
		break;
	case INST_BIND:
		printf("'%s'\n", inst.arg0.name);
		break;
	case INST_PUSH:
		print_item(inst.arg0.item);
		break;
	case INST_JZ:
	case INST_JNZ:
	case INST_JUMP:
		printf("%d\n", inst.arg0.jmp_mark);
		break;
	default:
		printf("\n");
		break;
	}
}

/*
 * VM
 */

void vm_init(VM * vm)
{
	stack_init(&vm->stack);
	vm->pc = 0;
	vm->instructions = 0;
	vm->symbol_table = make_map(512);
}

// Returns true while not halted
bool vm_step(VM * vm)
{
	Instruction inst = vm->instructions[vm->pc++];
	switch (inst.type) {
	case INST_JUMP:
		vm->pc = inst.arg0.jmp_mark;
		break;
	case INST_JZ: {
		Item top = spop();
		if (top.type != ITEM_LITERAL) {
			internal_error("The VM tried to compare something that's not a literal");
		}
		if (top.literal.val == 0) vm->pc = inst.arg0.jmp_mark;
	} break;
	case INST_JNZ: {
		Item top = spop();
		if (top.type != ITEM_LITERAL) {
			internal_error("The VM tried to compare something that's not a literal");
		}
		if (top.literal.val != 0) vm->pc = inst.arg0.jmp_mark;
	} break;
	case INST_RESOLVE: {
		Item top = spop();
		if (top.type != ITEM_VARIABLE) {
			internal_error("The VM tried to resolve something that's not a variable");
		}
		u64 recieve;
		if (!map_index(vm->symbol_table, (u64) top.variable.name, &recieve)) {
			runtime_line(inst.line, "Name '%s' is not bound.", top.variable.name);
		}
		Item new_top;
		new_top.type = ITEM_LITERAL;
		new_top.literal.val = (s64) recieve;
		spush(new_top);
	} break;
	case INST_BIND: {
		Item top = spop();
		if (top.type != ITEM_LITERAL) {
			internal_error("The VM tried to bind something that's not a literal");
		}
		const char * name = inst.arg0.name;
		map_insert(vm->symbol_table, (u64) inst.arg0.name, (u64) top.literal.val);
	} break;
	case INST_PUSH:
		spush(inst.arg0.item);
		break;
	case INST_POP:
		spop();
		break;
	case INST_OPERATOR:
		operators[inst.arg0.op_type](vm);
		break;
	case INST_PRINT: {
		Item top = spop();
		if (top.type != ITEM_LITERAL) {
			internal_error("The VM tried to print something that's not a literal");
		}
		printf("%ld\n", top.literal.val);
	} break;
	case INST_HALT:
		return false;
	default:
		internal_error("The VM read an instruction it could not execute");
		break;
	}
	return true;
}

void vm_print_state(VM * vm)
{
	printf("---------\n");
	printf("PC: %d\n", vm->pc);
	printf("Stack:\n");
	size_t stack_size = sb_count(vm->stack.arr);
	for (int i = 0; i < stack_size; i++) {
		printf("%03d ", stack_size - i - 1);
		print_item(stack_at(&vm->stack, i));
	}
	printf("Next Inst: ");
	print_instruction(vm->instructions[vm->pc]);
}

/*
 * Operators
 */

#define UNARY_OPERATOR(_OP_) \
	Item a = spop(); \
	if (a.type != ITEM_LITERAL) { \
		internal_error("Non-literals were plugged into a unary operator"); \
	} \
	Item c = {ITEM_LITERAL}; \
	c.literal.val = _OP_ a.literal.val; \
	spush(c);

#define BINARY_OPERATOR(_OP_)	\
	Item b = spop(); \
	Item a = spop(); \
	if (b.type != ITEM_LITERAL || a.type != ITEM_LITERAL) { \
		internal_error("Non-literals were plugged into a binary operator"); \
	} \
	Item c = {ITEM_LITERAL}; \
	c.literal.val = a.literal.val _OP_ b.literal.val;	\
	spush(c);

void operator_neg(VM * vm) { UNARY_OPERATOR(-);   }
void operator_add(VM * vm) { BINARY_OPERATOR(+);  }
void operator_sub(VM * vm) { BINARY_OPERATOR(-);  }
void operator_mul(VM * vm) { BINARY_OPERATOR(*);  }
void operator_div(VM * vm) { BINARY_OPERATOR(/);  }
void operator_eq (VM * vm) { BINARY_OPERATOR(==); }
void operator_gt (VM * vm) { BINARY_OPERATOR(>);  }
void operator_lt (VM * vm) { BINARY_OPERATOR(<);  }
void operator_gte(VM * vm) { BINARY_OPERATOR(>=); }
void operator_lte(VM * vm) { BINARY_OPERATOR(<=); }

void (*operators[])(VM*) = {
	[OP_NEG] = operator_neg,
	[OP_ADD] = operator_add,
	[OP_SUB] = operator_sub,
	[OP_MUL] = operator_mul,
	[OP_DIV] = operator_div,
	[OP_EQ]  = operator_eq,
	[OP_GT]  = operator_gt,
	[OP_LT]  = operator_lt,
	[OP_GTE] = operator_gte,
	[OP_LTE] = operator_lte,
};


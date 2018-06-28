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
 * VM
 */

#define sat(x)   (stack_at(&vm->stack, (x)))
#define spop()   (stack_pop(&vm->stack))
#define spush(x) (stack_push(&vm->stack, (x)))

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
	case INST_RESOLVE:
		
		break;
	case INST_OPERATOR:
		operators[inst.arg0.op_type](vm);
		break;
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
	printf("pc: %d\n", vm->pc);
	printf("STACK:\n");
	for (int i = 0; i < sb_count(vm->stack.arr); i++) {
		print_item(stack_at(&vm->stack, i));
	}
}

/*
 * Operators
 */

#define BINARY_OPERATOR_INIT \
	Item b = spop(); \
	Item a = spop(); \
	if (b.type != ITEM_LITERAL || a.type != ITEM_LITERAL) { \
		internal_error("Non-literals were plugged into an operator"); \
	} \
	Item c = {ITEM_LITERAL};

void operator_neg(VM * vm)
{
	Item a = spop();
	if (a.type != ITEM_LITERAL) {
		internal_error("Non-literals were plugged into an operator");
	}
	Item c = {ITEM_LITERAL};
	c.literal.val = -a.literal.val;
	spush(c);
}

void operator_add(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = a.literal.val + b.literal.val;
	spush(c);
}

void operator_sub(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = a.literal.val - b.literal.val;
	spush(c);
}

void operator_mul(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = a.literal.val * b.literal.val;
	spush(c);
}

void operator_div(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = a.literal.val / b.literal.val;
	spush(c);
}

void operator_eq(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = (int) (a.literal.val == b.literal.val);
	spush(c);
}

void operator_gt(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = (int) (a.literal.val > b.literal.val);
	spush(c);
}

void operator_lt(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = (int) (a.literal.val < b.literal.val);
	spush(c);
}

void operator_gte(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = (int) (a.literal.val >= b.literal.val);
	spush(c);
}

void operator_lte(VM * vm)
{
	BINARY_OPERATOR_INIT;
	c.literal.val = (int) (a.literal.val <= b.literal.val);
	spush(c);
}

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

void vm_test()
{
	VM _vm;
	VM * vm = &_vm;
	vm_init(vm);
	spush(((Item){ITEM_LITERAL, 5}));
	spush(((Item){ITEM_LITERAL, 10}));
	sb_push(vm->instructions, ((Instruction){INST_OPERATOR, OP_MUL}));
	sb_push(vm->instructions, ((Instruction){INST_HALT}));
	do {
		vm_print_state(vm);
	} while (vm_step(vm));
}

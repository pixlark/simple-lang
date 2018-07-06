#include "vm.h"

#define UNARY_OPERATOR(_OP_) \
	s64 x = vm->op_stack[--vm->op_sp]; \
	vm->op_stack[vm->op_sp++] = (_OP_ x);

#define BINARY_OPERATOR(_OP_)	\
	s64 y = vm->op_stack[--vm->op_sp]; \
	s64 x = vm->op_stack[--vm->op_sp]; \
	vm->op_stack[vm->op_sp++] = (x _OP_ y);

void operator_neg(VM * vm) { UNARY_OPERATOR(-);   }
void operator_add(VM * vm) { BINARY_OPERATOR(+);  }
void operator_sub(VM * vm) { BINARY_OPERATOR(-);  }
void operator_mul(VM * vm) { BINARY_OPERATOR(*);  }
void operator_div(VM * vm) { BINARY_OPERATOR(/);  }
void operator_mod(VM * vm) { BINARY_OPERATOR(%);  }
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
	[OP_MOD] = operator_mod,
	[OP_EQ]  = operator_eq,
	[OP_GT]  = operator_gt,
	[OP_LT]  = operator_lt,
	[OP_GTE] = operator_gte,
	[OP_LTE] = operator_lte,
};

#undef BINARY_OPERATOR
#undef UNARY_OPERATOR

char * inst_type_to_str[] = {
	[INST_HALT]  = "HALT",
	[INST_NOP]   = "NOP",
	[INST_SYMBOL] = "SYMBOL",
	[INST_OP]    = "OP",
	[INST_PUSHC] = "PUSHC",
	[INST_POPC]  = "POPC",
	[INST_PUSHO] = "PUSHO",
	[INST_POPO]  = "POPO",
	[INST_LOAD]  = "LOAD",
	[INST_SAVE]  = "SAVE",
	[INST_JMP]   = "JMP",
	[INST_JZ]    = "JZ",
	[INST_JNZ]   = "JNZ",
	[INST_JIP]   = "JIP",
	[INST_JSIP]  = "JSIP",
	[INST_PRNT]  = "PRNT",
};

void print_instruction(Inst inst)
{
	printf("%s ", inst_type_to_str[inst.type]);
	switch (inst.type) {
	case INST_OP:
		printf("%s\n", op_to_str[inst.arg0.op_type]);
		break;
	case INST_JMP:
	case INST_JZ:
	case INST_JNZ:
	case INST_JSIP:
		printf("%lu\n", inst.arg0.jmp_ip);
		break;
	case INST_LOAD:
	case INST_SAVE:
		printf("%lu\n", inst.arg0.offset);
		break;
	case INST_PUSHC:
	case INST_PUSHO:
		printf("%ld\n", inst.arg0.literal);
		break;
	default:
		printf("\n");
		break;
	}
}

int vm_init(VM * vm)
{
	vm->op_sp   = 0;
	vm->call_sp = 0;
	vm->ip      = 0;
	vm->insts   = NULL;
}

bool vm_step(VM * vm)
{
	Inst inst = vm->insts[vm->ip++];
	switch (inst.type) {
	case INST_HALT:
		return false;
	case INST_NOP:
	case INST_SYMBOL:
		break;
	case INST_OP:
		operators[inst.arg0.op_type](vm);
		break;
	case INST_PUSHC:
		vm->call_stack[vm->call_sp++] = (s64) inst.arg0.literal;
		break;
	case INST_POPC:
		if (vm->call_sp == 0)
			internal_error("POPC executed with an empty call stack");
		vm->call_sp--;
		break;
	case INST_PUSHO:
		vm->op_stack[vm->op_sp++] = (s64) inst.arg0.literal;
		break;
	case INST_POPO:
		if (vm->op_sp == 0)
			internal_error("PUSHO executed with an empty op stack");
		vm->op_sp--;
		break;
	case INST_LOAD:
		if (inst.arg0.offset < 1)
			internal_error("Tried to load from past call stack");
		if (inst.arg0.offset > vm->call_sp)
			internal_error("Tried to load from before call stack");
		vm->op_stack[vm->op_sp++] =
			vm->call_stack[vm->call_sp - inst.arg0.offset];
		break;
	case INST_SAVE:
		if (vm->op_sp == 0)
			internal_error("SAVE executed with an empty op stack");
		if (inst.arg0.offset < 1)
			internal_error("Tried to save past call stack");
		if (inst.arg0.offset > vm->call_sp)
			internal_error("Tried to save before call stack");
		vm->call_stack[vm->call_sp - inst.arg0.offset] =
			vm->op_stack[--vm->op_sp];
		break;
	case INST_JMP:
	jump:
		vm->ip = inst.arg0.jmp_ip;
		break;
	case INST_JZ: {
		s64 pop = vm->op_stack[--vm->op_sp];
		if (pop == 0) goto jump;
	} break;
	case INST_JNZ: {
		s64 pop = vm->op_stack[--vm->op_sp];
		if (pop != 0) goto jump;
	} break;
	case INST_PRNT: {
		s64 pop = vm->op_stack[vm->op_sp];
		printf("~~~~| %ld |~~~~\n", pop);
	} break;
	case INST_JIP: {
		u64 pop = (u64) vm->op_stack[--vm->op_sp];
		vm->ip = pop;
	} break;
	case INST_JSIP: {
		vm->call_stack[vm->call_sp++] = vm->ip;
		goto jump;
	} break;
	default:
		internal_error("VM read invalid instruction");
		break;
	}
	return true;
}

void vm_test()
{
	VM _vm;
	VM * vm = &_vm;
	vm_init(vm);

	// add procedure
	EMIT_ARG(INST_PUSHC, literal, 0);   // 0
	EMIT_ARG(INST_LOAD, offset, 4);
	EMIT_ARG(INST_LOAD, offset, 3);
	EMIT_ARG(INST_OP, op_type, OP_ADD);
	EMIT_ARG(INST_SAVE, offset, 1);     // 4
	EMIT_ARG(INST_LOAD, offset, 1);
	EMIT_ARG(INST_LOAD, offset, 2);
	EMIT(INST_POPC);
	EMIT(INST_JIP);                     // 8

	vm->ip = 9;
	// main
	EMIT_ARG(INST_PUSHC, literal, 1);
	EMIT_ARG(INST_PUSHC, literal, 2);
	EMIT_ARG(INST_JSIP, jmp_ip, 0);
	EMIT(INST_POPC);                    // 12
	EMIT(INST_POPC);
	EMIT(INST_POPC);
	EMIT(INST_HALT);
	
	/*
	EMIT_ARG(INST_PUSHC, literal, 12);
	EMIT_ARG(INST_LOAD, offset, 1);
	EMIT_ARG(INST_PUSHO, literal, 3);
	EMIT_ARG(INST_OP, op_type, OP_ADD);
	EMIT_ARG(INST_SAVE, offset, 1);
	EMIT(INST_POPC);
	EMIT(INST_HALT);*/

	#define CYCLE_LIMIT 100
	int cycles = 0;
	do {
		#if VM_TEST_DEBUG
		printf("----\n");
		printf("IP: %d\n", vm->ip);
		printf("Call Stack (%lu):\n", vm->call_sp);
		if (vm->call_sp == 0) printf(" - \n");
		for (int i = vm->call_sp - 1; i >= 0; i--) {
			printf(" %ld\n", vm->call_stack[i]);
		}
		printf("Op Stack (%lu):\n", vm->op_sp);
		if (vm->op_sp == 0) printf(" - \n");
		for (int i = vm->op_sp - 1; i >= 0; i--) {
			printf(" %ld\n", vm->op_stack[i]);
		}
		printf("%s\n", inst_type_to_str[vm->insts[vm->ip].type]);
		#endif
		cycles++;
	} while (vm_step(vm) && cycles < CYCLE_LIMIT);
	#if VM_TEST_DEBUG
	printf("--------\n");
	#endif
}


#include "compiler.h"

#include "lexer.h"

void compile_expression(VM * vm, Expression * expr)
{
	switch (expr->type) {
	case EXPR_UNARY:
		compile_expression(vm, expr->unary.right);
		ipush(((Instruction){INST_OPERATOR, .arg0.op_type =
						expr->unary.type}));
		break;
	case EXPR_BINARY:
		compile_expression(vm, expr->binary.left);
		compile_expression(vm, expr->binary.right);
		ipush(((Instruction){INST_OPERATOR, .arg0.op_type =
						expr->binary.type}));
		break;
	case EXPR_NAME:
		ipush(((Instruction){INST_PUSH,
						(Item){ITEM_VARIABLE, .variable.name =
							expr->name.name}}));
		ipush(((Instruction){INST_RESOLVE}));
		break;
	case EXPR_LITERAL:
		ipush(((Instruction){INST_PUSH,
						(Item){ITEM_LITERAL, .literal.val =
							expr->literal.value}}));
		break;
	}
}

void compile_statement(VM * vm, Statement * stmt)
{
	switch (stmt->type) {
	case STMT_LET:
		compile_expression(vm, stmt->stmt_let.bind_val);
		ipush(((Instruction){INST_BIND,
						.arg0.name = stmt->stmt_let.bind_name}));
		break;
	case STMT_WHILE: {
		u32 loop_mark = sb_count(vm->instructions);
		compile_expression(vm, stmt->stmt_if.condition);
		u32 exit_place = sb_count(vm->instructions);
		ipush(((Instruction){INST_JZ}));
		assert(vm->instructions[exit_place].type == INST_JZ);
		for (int i = 0; i < sb_count(stmt->stmt_if.body); i++) {
			compile_statement(vm, stmt->stmt_if.body[i]);
		}
		ipush(((Instruction){INST_JUMP,
						.arg0.jmp_mark = loop_mark}));
		vm->instructions[exit_place].arg0.jmp_mark =
			sb_count(vm->instructions);
	} break;
	case STMT_IF: {
		compile_expression(vm, stmt->stmt_if.condition);
		u32 jmp_place = sb_count(vm->instructions);
		ipush(((Instruction){INST_JZ}));
		assert(vm->instructions[jmp_place].type == INST_JZ);
		for (int i = 0; i < sb_count(stmt->stmt_if.body); i++) {
			compile_statement(vm, stmt->stmt_if.body[i]);
		}
		vm->instructions[jmp_place].arg0.jmp_mark =
			sb_count(vm->instructions);
	} break;
	case STMT_PRINT: {
		compile_expression(vm, stmt->stmt_print.to_print);
		ipush(((Instruction){INST_PRINT}));
	} break;
	}
}

void compile_test()
{
	VM _vm;
	VM * vm = &_vm;
	vm_init(vm);
	
	const char * source = load_string_from_file("fibonacci.sl");
	
	#if COMPILE_TEST_DEBUG
	printf("-------\nSOURCE\n-------\n");
	printf("%s\n", source);
	#endif
	
	init_stream(source);
	
	#if COMPILE_TEST_DEBUG
	printf("-------\nAST\n-------\n");
	#endif
	
	while (token.type) {
		Statement * stmt = parse_statement();
		
		#if COMPILE_TEST_DEBUG
		print_statement(stmt);
		printf("\n");
		#endif
		
		compile_statement(vm, stmt);
	}
	ipush(((Instruction){INST_HALT}));

	#if COMPILE_TEST_DEBUG
	printf("-------\nINSTRUCTIONS\n-------\n");
	for (int i = 0; i < sb_count(vm->instructions); i++) {
		printf("%03d ", i);
		print_instruction(vm->instructions[i]);
	}
	#endif

	do {
		#if CPU_STATE_REPORTING
		vm_print_state(vm);
		#endif
	} while (vm_step(vm));

	#if COMPILE_TEST_DEBUG
	printf("-------\nEND VARIABLE STATE\n-------\n");
	for (int i = 0; i < 512; i++) {
		if (vm->symbol_table->taken[i]) {
			printf("%s = %ld\n", (char*) vm->symbol_table->keys[i], (s64) vm->symbol_table->values[i]);
		}
	}
	#endif
}
#include "compiler.h"

#include "lexer.h"

#if 0
void compile_expression(VM * vm, Expression * expr)
{
	switch (expr->type) {
	case EXPR_UNARY:
		compile_expression(vm, expr->unary.right);
		ipush(((Instruction){INST_OPERATOR, .arg0.op_type =
						expr->unary.type}), expr->line);
		break;
	case EXPR_BINARY:
		compile_expression(vm, expr->binary.left);
		compile_expression(vm, expr->binary.right);
		ipush(((Instruction){INST_OPERATOR, .arg0.op_type =
						expr->binary.type}), expr->line);
		break;
	case EXPR_NAME:
		ipush(((Instruction){INST_PUSH,
						(Item){ITEM_VARIABLE, .variable.name =
							expr->name.name}}), expr->line);
		ipush(((Instruction){INST_RESOLVE}), expr->line);
		break;
	case EXPR_LITERAL:
		ipush(((Instruction){INST_PUSH,
						(Item){ITEM_LITERAL, .literal.val =
							expr->literal.value}}), expr->line);
		break;
	}
}

void compile_statement(VM * vm, Statement * stmt)
{
	switch (stmt->type) {
	case STMT_LET:
		compile_expression(vm, stmt->stmt_let.bind_val);
		ipush(((Instruction){INST_BIND,
						.arg0.name = stmt->stmt_let.bind_name}), stmt->line);
		break;
	case STMT_WHILE: {
		u32 loop_mark = sb_count(vm->instructions);
		compile_expression(vm, stmt->stmt_if.condition);
		u32 exit_place = sb_count(vm->instructions);
		ipush(((Instruction){INST_JZ}), stmt->line);
		assert(vm->instructions[exit_place].type == INST_JZ);
		for (int i = 0; i < sb_count(stmt->stmt_if.body); i++) {
			compile_statement(vm, stmt->stmt_if.body[i]);
		}
		ipush(((Instruction){INST_JUMP,
						.arg0.jmp_mark = loop_mark}), stmt->line);
		vm->instructions[exit_place].arg0.jmp_mark =
			sb_count(vm->instructions);
	} break;
	case STMT_IF: {
		/* Without else:
		 * 0 CONDITION
		 * 1 JZ 4
		 * 2 BODY
		 * 3 JMP 4
		 * 4 ...
		 */
		/* With else:
		 * 0 CONDITION
		 * 1 JZ 4
		 * 2 BODY
		 * 3 JMP 5
		 * 4 ELSE_BODY
		 * 5 ...
		 */
		compile_expression(vm, stmt->stmt_if.condition);
		// If else is triggered, this jump is followed
		u32 jz_place = sb_count(vm->instructions);
		ipush(((Instruction){INST_JZ}), stmt->line);
		assert(vm->instructions[jz_place].type == INST_JZ);

		// Compile body
		for (int i = 0; i < sb_count(stmt->stmt_if.body); i++) {
			compile_statement(vm, stmt->stmt_if.body[i]);
		}

		// If main condition is triggered, this jump is followed after
		// execution of body
		u32 jump_place = sb_count(vm->instructions);
		ipush(((Instruction){INST_JUMP}), stmt->line);
		assert(vm->instructions[jump_place].type == INST_JUMP);

		// This is where else jumps to
		vm->instructions[jz_place].arg0.jmp_mark =
			sb_count(vm->instructions);
		
		if (stmt->stmt_if.has_else) {
			for (int i = 0; i < sb_count(stmt->stmt_if.else_body); i++) {
				compile_statement(vm, stmt->stmt_if.else_body[i]);
			}
		}

		// This is where main condition jumps to after body
		vm->instructions[jump_place].arg0.jmp_mark =
			sb_count(vm->instructions);
	} break;
	case STMT_PRINT: {
		compile_expression(vm, stmt->stmt_print.to_print);
		ipush(((Instruction){INST_PRINT}), stmt->line);
	} break;
	}
}
#endif

void decl_pass(Function * function)
{
	
}

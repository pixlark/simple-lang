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

void compile_expression(VM * vm, Expression * expr)
{
	switch (expr->type) {
	case EXPR_UNARY:
		compile_expression(vm, expr->unary.right);
		EMIT_ARG(INST_OP, op_type, expr->unary.type);
		break;
	case EXPR_BINARY:
		compile_expression(vm, expr->binary.left);
		compile_expression(vm, expr->binary.right);
		EMIT_ARG(INST_OP, op_type, expr->binary.type);
		break;
	case EXPR_INDEX:
		internal_error("Indexing operator not yet supported");
		break;
	case EXPR_FUNCALL:
		break;
	case EXPR_NAME:
		break;
	case EXPR_LITERAL:
		EMIT_ARG(INST_PUSHO, literal, expr->literal.value);
		break;
	}
}

void compile_statement(VM * vm, Statement * stmt)
{
	switch (stmt->type) {
	case STMT_EXPR:
		compile_expression(vm, stmt->stmt_expr.expr);
		EMIT(INST_POPO);
		break;
	case STMT_ASSIGN:
		break;
	case STMT_DECL:
		break;
	case STMT_IF: {
		/* 0 CONDITION 0
		 * 1 JZ 4
		 * 2 BODY 0
		 * 3 JMP 9
		 * 4 CONDITION 1
		 * 5 JZ 8
		 * 6 BODY
		 * 7 JMP 9
		 * 8 ELSE_BODY
		 */
		assert(sb_count(stmt->stmt_if.conditions) == sb_count(stmt->stmt_if.scopes));
		int * jmps = 0;
		for (int i = 0; i < sb_count(stmt->stmt_if.conditions); i++) {
			compile_expression(vm, stmt->stmt_if.conditions[i]);
			int jz = sb_count(vm->insts);
			EMIT(INST_JZ);
			compile_statement(vm, stmt->stmt_if.scopes[i]);
			sb_push(jmps, sb_count(vm->insts));
			EMIT(INST_JMP);
			vm->insts[jz].arg0.jmp_ip = sb_count(vm->insts);
		}
		if (stmt->stmt_if.else_scope) {
			compile_statement(vm, stmt->stmt_if.else_scope);
		}
		for (int i = 0; i < sb_count(jmps); i++) {
			vm->insts[jmps[i]].arg0.jmp_ip = sb_count(vm->insts);
		}
	} break;
	case STMT_WHILE:
		break;
	case STMT_RETURN:
		break;
	case STMT_SCOPE:
		for (int i = 0; i < sb_count(stmt->stmt_scope.body); i++) {
			compile_statement(vm, stmt->stmt_scope.body[i]);
		}
		break;
	}
}

void compile_function(VM * vm, Function * func)
{
	func->ip_start = sb_count(vm->insts);
	compile_statement(vm, func->body);
}

void tag_names_in_expr(Expression * expr, const char * name, int pos)
{
	switch (expr->type) {
	case EXPR_UNARY:
		tag_names_in_expr(expr->unary.right, name, pos);
		break;
	case EXPR_INDEX:
		tag_names_in_expr(expr->index.left, name, pos);
		tag_names_in_expr(expr->index.right, name, pos);
		break;
	case EXPR_FUNCALL:
		tag_names_in_expr(expr->funcall.name, name, pos);
		for (int i = 0; i < sb_count(expr->funcall.args); i++) {
			tag_names_in_expr(expr->funcall.args[i], name, pos);
		}
		break;
	case EXPR_BINARY:
		tag_names_in_expr(expr->binary.left, name, pos);
		tag_names_in_expr(expr->binary.right, name, pos);
		break;
	case EXPR_NAME:
		if (expr->name.name == name) {
			printf("Tagged %s\n", name);
			expr->name.decl_pos = pos;
		}
		break;
	}
}

void tag_names(Statement * scope, int start, const char * name, int pos)
{
	assert(scope->type == STMT_SCOPE);
	for (int i = start; i < sb_count(scope->stmt_scope.body); i++) {
		Statement * it = scope->stmt_scope.body[i];
		switch (it->type) {
		case STMT_EXPR:
			tag_names_in_expr(it->stmt_expr.expr, name, pos);
			break;
		case STMT_ASSIGN:
			tag_names_in_expr(it->stmt_assign.left, name, pos);
			tag_names_in_expr(it->stmt_assign.right, name, pos);
			break;
		case STMT_IF:
			// Tag names in conditions
			for (int i = 0; i < sb_count(it->stmt_if.conditions); i++) {
				tag_names_in_expr(it->stmt_if.conditions[i], name, pos);
			}
			// Recurse with each if scope
			for (int i = 0; i < sb_count(it->stmt_if.scopes); i++) {
				tag_names(it->stmt_if.scopes[i], 0, name, pos);
			}
			if (it->stmt_if.else_scope) {
				tag_names(it->stmt_if.else_scope, 0, name, pos);
			}
			break;
		case STMT_WHILE:
			// Tag names in condition
			tag_names_in_expr(it->stmt_while.condition, name, pos);
			// Recurse with the while scope
			tag_names(it->stmt_while.scope, 0, name, pos);
			break;
		case STMT_RETURN:
			tag_names_in_expr(it->stmt_return.expr, name, pos);
			break;
		case STMT_SCOPE:
			tag_names(it, 0, name, pos);
			break;
		}
	}
}

void read_declarations(Declaration ** decls, Statement * stmt)
{
	switch (stmt->type) {
	case STMT_IF:
		// Recurse with each if scope
		for (int i = 0; i < sb_count(stmt->stmt_if.scopes); i++) {
			read_declarations(decls, stmt->stmt_if.scopes[i]);
		}
		if (stmt->stmt_if.else_scope) {
			read_declarations(decls, stmt->stmt_if.else_scope);
		}
		break;
	case STMT_WHILE:
		// Recurse with the while scope
		read_declarations(decls, stmt->stmt_while.scope);
		break;
	case STMT_SCOPE:
		for (int i = 0; i < sb_count(stmt->stmt_scope.body); i++) {
			Statement * it = stmt->stmt_scope.body[i];
			if (it->type == STMT_DECL) {
				printf("Declaration: %s\n", it->stmt_decl.name);
				Declaration decl;
				decl.name = it->stmt_decl.name;
				decl.size = sizeof(u64); // No types at the moment
				decl.decl_pos = sb_count(*decls);
				tag_names(stmt, i, it->stmt_decl.name, sb_count(*decls));
				sb_push(*decls, decl);
			} else {
				read_declarations(decls, it);
			}
		}
		break;
	}
}

Declaration * read_function_decls(Function * func)
{
	Declaration * decls = 0;
	read_declarations(&decls, func->body);
	return decls;
}


void prepare_function(Function * func)
{
	func->decls = read_function_decls(func);
}

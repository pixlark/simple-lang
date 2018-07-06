#include "compiler.h"

#include "lexer.h"

Map * function_map;
int * return_jumps; // TODO(pixlark): Hacky global variable. Fix this.

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
	case EXPR_FUNCALL: {
		for (int i = 0; i < sb_count(expr->funcall.args); i++) {
			EMIT_ARG(INST_PUSHC, literal, 0); // Make space for argument
			compile_expression(vm, expr->funcall.args[i]);
			EMIT_ARG(INST_SAVE, offset, 1);   // Save arg into space
		}
		const char * name = expr->funcall.name->name.name;
		if (!map_index(function_map, (u64) name, NULL)) {
			fatal("Function %s does not exist", name);
		}
		Function * func;
		map_index(function_map, (u64) name, (u64*) &func);
		EMIT_ARG(INST_JSIP, jmp_ip, func->ip_start);
		EMIT(INST_POPC); // Pop ip
		for (int i = 0; i < sb_count(expr->funcall.args); i++)
			EMIT(INST_POPC); // Pop args
	} break;
	case EXPR_NAME:
		if (expr->name.decl_pos == -1) {
			internal_error("Encountered untagged name %s", expr->name.name);
		}
		EMIT_ARG(INST_LOAD, offset, expr->name.decl_pos);
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
		if (stmt->stmt_assign.left->type != EXPR_NAME) {
			internal_error("All lvalues are bare names at the moment");
		}
		compile_expression(vm, stmt->stmt_assign.right);
		EMIT_ARG(INST_SAVE, offset, stmt->stmt_assign.left->name.decl_pos);
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
	case STMT_WHILE: {
		int begin = sb_count(vm->insts);
		compile_expression(vm, stmt->stmt_while.condition);
		int jz_end = sb_count(vm->insts);
		EMIT(INST_JZ);
		compile_statement(vm, stmt->stmt_while.scope);
		EMIT_ARG(INST_JMP, jmp_ip, begin);
		vm->insts[jz_end].arg0.jmp_ip = sb_count(vm->insts);
	} break;
	case STMT_RETURN:
		compile_expression(vm, stmt->stmt_return.expr);
		sb_push(return_jumps, sb_count(vm->insts));
		EMIT(INST_JMP);
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
	return_jumps = 0;
	func->ip_start = sb_count(vm->insts);
	EMIT_ARG(INST_SYMBOL, symbol, func->name);
	for (int i = 0; i < sb_count(func->decls); i++) {
		EMIT_ARG(INST_PUSHC, literal, 0);
	}
	compile_statement(vm, func->body);
	for (int i = 0; i < sb_count(return_jumps); i++) {
		if (vm->insts[return_jumps[i]].type != INST_JMP) {
			internal_error("Invalid instruction in return_jumps");
		}
		vm->insts[return_jumps[i]].arg0.jmp_ip = sb_count(vm->insts);
	}
	for (int i = 0; i < sb_count(func->decls); i++) {
		EMIT(INST_POPC);
	}
	EMIT_ARG(INST_LOAD, offset, 1);
	EMIT(INST_JIP);
}

void compile(VM * vm)
{
	int iter = -1;
	while ((iter = map_iter(function_map, iter)) != -1) {
		printf("Compiling function %s\n",
			((Function*)function_map->values[iter])->name);
		compile_function(vm,
			(Function*) function_map->values[iter]);
	}
	vm->ip = sb_count(vm->insts);
	if (!map_index(function_map, (u64) str_intern("main"), NULL)) {
		fatal("No main function");
	}
	Function * main;
	map_index(function_map, (u64) str_intern("main"), (u64*) &main);
	EMIT_ARG(INST_JSIP, jmp_ip, main->ip_start);
	EMIT(INST_POPC);
	EMIT(INST_HALT);
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
				Declaration decl;
				decl.name = it->stmt_decl.name;
				decl.size = sizeof(u64); // No types at the moment
				decl.decl_pos = sb_count(*decls);
				tag_names(stmt, i, it->stmt_decl.name, sb_count(*decls) + 1);
				sb_push(*decls, decl);
			} else {
				read_declarations(decls, it);
			}
		}
		break;
	}
}

void tag_args(Function * func)
{
	int decl_count = sb_count(func->decls);
	int arg_count  = sb_count(func->arg_names);
	for (int i = 0; i < arg_count; i++) {
		tag_names(func->body, 0, func->arg_names[i],
			decl_count + 1 + (arg_count - i));
	}
}

Declaration * read_function_decls(Function * func)
{
	Declaration * decls = 0;
	read_declarations(&decls, func->body);
	return decls;
}

void prepare()
{
	function_map = make_map(512);
	while (tokens_left()) {
		Function * func = parse_function();
		prepare_function(func);
		map_insert(function_map, (u64) func->name, (u64) func);
	}
}

void prepare_function(Function * func)
{
	func->decls = read_function_decls(func);
	tag_args(func);
}

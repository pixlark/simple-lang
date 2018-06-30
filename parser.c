#include "parser.h"

char * op_to_str[] = {
	[OP_NEG]  = "NEG",
	[OP_LNEG] = "!",
	[OP_ADD]  = "+",
	[OP_SUB]  = "-",
	[OP_MUL]  = "*",
	[OP_DIV]  = "/",
	[OP_MOD]  = "%",
	[OP_EQ]   = "==",
	[OP_GT]   = ">",
	[OP_LT]   = "<",
	[OP_GTE]  = ">=",
	[OP_LTE]  = "<=",
};

Operator_Type token_to_bin_op[] = {
	['+']       = OP_ADD,
	['-']       = OP_SUB,
	['*']       = OP_MUL,
	['/']       = OP_DIV,
	['%']       = OP_MOD,
	[TOKEN_EQ]  = OP_EQ,
	['>']       = OP_GT,
	['<']       = OP_LT,
	[TOKEN_GTE] = OP_GTE,
	[TOKEN_LTE] = OP_LTE,
};

#if 0
Statement * make_stmt(Stmt_Type type, u32 line)
{
	Statement * stmt = (Statement*) malloc(sizeof(Statement));
	stmt->type = type;
	// Necessary initialization (stretchy buffers, etc)
	switch (type) {
	case STMT_WHILE:
		stmt->stmt_while.body = NULL;
		break;
	case STMT_IF:
		stmt->stmt_if.body = NULL;
		stmt->stmt_if.else_body = NULL;
		break;
	}
	stmt->line = line;
	return stmt;
}

Expression * make_expr(Expr_Type type, u32 line)
{
	Expression * expr = (Expression*) malloc(sizeof(Expression));
	expr->line = line;
	expr->type = type;
	return expr;
}

void print_expression(Expression * expr)
{
	switch (expr->type) {
	case EXPR_UNARY:
		printf("(%s ", op_to_str[expr->unary.type]);
		print_expression(expr->unary.right);
		printf(")");
		break;
	case EXPR_BINARY:
		printf("(%s ", op_to_str[expr->binary.type]);
		print_expression(expr->binary.left);
		printf(" ");
		print_expression(expr->binary.right);
		printf(")");
		break;
	case EXPR_NAME:
		printf("%s", expr->name.name);
		break;
	case EXPR_LITERAL:
		printf("%lu", expr->literal.value);
		break;
	}
}

void print_statement(Statement * stmt)
{
	switch (stmt->type) {
	case STMT_LET:
		printf("(let ");
		printf("%s ", stmt->stmt_let.bind_name);
		print_expression(stmt->stmt_let.bind_val);
		printf(")");
		break;
	case STMT_WHILE:
		printf("(while ");
		print_expression(stmt->stmt_while.condition);
		printf(" ");
		for (int i = 0; i < sb_count(stmt->stmt_while.body); i++) {
			print_statement(stmt->stmt_while.body[i]);
			if (i != sb_count(stmt->stmt_while.body) - 1)
				printf(" ");
		}
		printf(")");
		break;
	case STMT_IF:
		printf("(if ");
		print_expression(stmt->stmt_if.condition);
		printf(" ");
		for (int i = 0; i < sb_count(stmt->stmt_if.body); i++) {
			print_statement(stmt->stmt_if.body[i]);
			if (i != sb_count(stmt->stmt_if.body) - 1)
				printf(" ");
		}
		printf(")");
		break;
	case STMT_PRINT:
		printf("(print ");
		print_expression(stmt->stmt_print.to_print);
		printf(")");
		break;
	}
}

Expression * parse_atom()
{
	Expression * expr;
	switch (token.type) {
	case TOKEN_LITERAL:
		expr = make_expr(EXPR_LITERAL, token.line);
		expr->literal.value = token.literal;
		next_token();
		break;
	case TOKEN_NAME:
		expr = make_expr(EXPR_NAME, token.line);
		expr->name.name = token.name;
		next_token();
		break;
	case '(':
		next_token();
		expr = parse_expression();
		expect_token(')');
		break;
	default: {
		char buf[256];
		token_type_str(buf, token.type);
		fatal_line(token.line, "Token %s is not valid inside expression.", buf);
	} break;
	}
	return expr;
}

Expression * parse_expr_0()
{
	Expression * expr;
	if (token.type == '-') {
		expr = make_expr(EXPR_UNARY, token.line);
		expr->unary.type = OP_NEG;
		next_token();
		expr->unary.right = parse_expr_0();
	} else {
		expr = parse_atom();
	}
	return expr;
}

bool is_token_expr_1()
{
	return is_token('*') || is_token('/') || is_token('%');
}

Expression * parse_expr_1()
{
	Expression * left = parse_expr_0();
	while (is_token_expr_1()) {
		Expression * expr = make_expr(EXPR_BINARY, token.line);
		expr->binary.left = left;
		expr->binary.type = token_to_bin_op[token.type];
		next_token();
		expr->binary.right = parse_expr_0();
		left = expr;
	}
	return left;
}

bool is_token_expr_2()
{
	return is_token('+') || is_token('-');
}

Expression * parse_expr_2()
{
	Expression * left = parse_expr_1();
	while (is_token_expr_2()) {
		Expression * expr = make_expr(EXPR_BINARY, token.line);
		expr->binary.left = left;
		expr->binary.type = token_to_bin_op[token.type];
		next_token();
		expr->binary.right = parse_expr_1();
		left = expr;
	}
	return left;
}

bool is_token_expr_3()
{
	return
		is_token(TOKEN_EQ) ||
		is_token('>') ||
		is_token('<') ||
		is_token(TOKEN_GTE) ||
		is_token(TOKEN_LTE);
}

Expression * parse_expr_3()
{
	Expression * left = parse_expr_2();
	while (is_token_expr_3()) {
		Expression * expr = make_expr(EXPR_BINARY, token.line);
		expr->binary.left = left;
		expr->binary.type = token_to_bin_op[token.type];
		next_token();
		expr->binary.right = parse_expr_2();
		left = expr;
	}
	return left;
}

Expression * parse_expression()
{
	return parse_expr_3();
}

Statement * parse_let()
{
	Statement * stmt = make_stmt(STMT_LET, token.line);
	check_token(TOKEN_NAME);
	stmt->stmt_let.bind_name = token.name;
	next_token();
	expect_token('=');
	stmt->stmt_let.bind_val = parse_expression();
}

Statement * parse_while()
{
	Statement * stmt = make_stmt(STMT_WHILE, token.line);
	stmt->stmt_while.condition = parse_expression();
	expect_token('{');
	while (!match_token('}')) {
		sb_push(stmt->stmt_while.body, parse_statement());
	}
	return stmt;
}

Statement * parse_if()
{
	Statement * stmt = make_stmt(STMT_IF, token.line);
	stmt->stmt_if.condition = parse_expression();
	expect_token('{');
	while (!match_token('}')) {
		sb_push(stmt->stmt_if.body, parse_statement());
	}
	if (match_token(TOKEN_ELSE)) {
		stmt->stmt_if.has_else = true;
		expect_token('{');
		while (!match_token('}')) {
			sb_push(stmt->stmt_if.else_body, parse_statement());
		}
	} else {
		stmt->stmt_if.has_else = false;
	}
	return stmt;
}

Statement * parse_print()
{
	Statement * stmt = make_stmt(STMT_PRINT, token.line);
	stmt->stmt_print.to_print = parse_expression();
	return stmt;
}

Statement * parse_statement()
{
	Statement * stmt;
	if (is_token(TOKEN_LET)) {
		next_token();
		stmt = parse_let();
	} else if (is_token(TOKEN_WHILE)) {
		next_token();
		stmt = parse_while();
	} else if (is_token(TOKEN_IF)) {
		next_token();
		stmt = parse_if();
	} else if (is_token(TOKEN_PRINT)) {
		next_token();
		stmt = parse_print();
	} else {
		fatal_line(token.line, "Not a statement");
	}
	expect_token(';');
	return stmt;
}
#endif

Expression * make_expr(Expr_Type type)
{
	Expression * expr = (Expression*) calloc(1, sizeof(Expression));
	expr->type = type;
	return expr;
}

Statement * make_stmt(Stmt_Type type)
{
	Statement * stmt = (Statement*) calloc(1, sizeof(Statement));
	stmt->type = type;
	return stmt;
}

void print_expression(Expression * expr)
{
	switch (expr->type) {
	case EXPR_UNARY:
		printf("(%s ", op_to_str[expr->unary.type]);
		print_expression(expr->unary.right);
		printf(")");
		break;
	case EXPR_BINARY:
		printf("(%s ", op_to_str[expr->binary.type]);
		print_expression(expr->binary.left);
		printf(" ");
		print_expression(expr->binary.right);
		printf(")");
		break;
	case EXPR_INDEX:
		printf("(index ");
		print_expression(expr->index.left);
		printf(" ");
		print_expression(expr->index.right);
		printf(")");
		break;
	case EXPR_FUNCALL:
		printf("(");
		print_expression(expr->funcall.name);
		for (int i = 0; i < sb_count(expr->funcall.args); i++) {
			printf(" ");
			print_expression(expr->funcall.args[i]);
		}
		printf(")");
		break;
	case EXPR_NAME:
		printf("%s", expr->name.name);
		break;
	case EXPR_LITERAL:
		printf("%lu", expr->literal.value);
		break;
	}
}

void print_statement(Statement * stmt)
{
	switch (stmt->type) {
	case STMT_EXPR:
		print_expression(stmt->stmt_expr.expr);
		break;
	case STMT_ASSIGN:
		printf("(set ");
		print_expression(stmt->stmt_assign.left);
		printf(" ");
		print_expression(stmt->stmt_assign.right);
		printf(")");
		break;
	case STMT_DECL:
		printf("(let %s ", stmt->stmt_decl.bind_name);
		print_expression(stmt->stmt_decl.bind_expr);
		printf(")");
		break;
	case STMT_IF:
		printf("(");
		assert(sb_count(stmt->stmt_if.conditions) == sb_count(stmt->stmt_if.scopes));
		for (int i = 0; i < sb_count(stmt->stmt_if.conditions); i++) {
			printf("%s ", i == 0 ? "if" : " elif");
			print_expression(stmt->stmt_if.conditions[i]);
			printf(" then ");
			print_statement(stmt->stmt_if.scopes[i]);
		}
		if (stmt->stmt_if.else_scope) {
			printf(" else ");
			print_statement(stmt->stmt_if.else_scope);
		}
		printf(")");
		break;
	case STMT_WHILE:
		printf("(while ");
		print_expression(stmt->stmt_while.condition);
		printf(" ");
		print_statement(stmt->stmt_while.scope);
		printf(")");
		break;
	case STMT_SCOPE:
		printf("(");
		for (int i = 0; i < sb_count(stmt->stmt_scope.body); i++) {
			print_statement(stmt->stmt_scope.body[i]);
			if (i != sb_count(stmt->stmt_scope.body) - 1) printf(" ");
		}
		printf(")");
		break;
	}
}

void print_function(Function * func)
{
	printf("(defun %s (", func->name);
	for (int i = 0; i < sb_count(func->arg_names); i++) {
		printf("%s%s", func->arg_names[i], i != sb_count(func->arg_names) - 1 ? " " : "");
	}
	printf(") ");
	print_statement(func->body);
	printf(")");
}

Expression * parse_atom()
{
	Expression * expr;
	switch (token.type) {
	case TOKEN_LITERAL:
		expr = make_expr(EXPR_LITERAL);
		expr->literal.value = token.literal;
		next_token();
		break;
	case TOKEN_NAME:
		expr = make_expr(EXPR_NAME);
		expr->name.name = token.name;
		next_token();
		break;
	case '(':
		next_token();
		expr = parse_expression();
		expect_token(')');
		break;
	default: {
		char buf[256];
		token_type_str(buf, token.type);
		fatal("Token %s is not valid inside expression.", buf);
	} break;
	}
	return expr;
}

// NOTE: Leading ( has already been consumed when calling this
Expression ** parse_arglist()
{
	Expression ** arglist = 0;
	if (!match_token(')')) {
		while (1) {
			sb_push(arglist, parse_expression());
			if (!match_token(',')) {
				expect_token(')');
				break;
			}
		}
	}
	return arglist;
}

Expression * parse_postfix()
{
	Expression * left = parse_atom();
	if (match_token('[')) {
		Expression * expr = make_expr(EXPR_INDEX);
		expr->index.left  = left;
		expr->index.right = parse_atom();
		expect_token(']');
		left = expr;
	} else if (match_token('(')) {
		Expression * expr = make_expr(EXPR_FUNCALL);
		expr->funcall.name = left;
		expr->funcall.args = parse_arglist();
		left = expr;
	}
	return left;
}

Expression * parse_prefix()
{
	if (match_token('-')) {
		Expression * expr = make_expr(EXPR_UNARY);
		expr->unary.type = OP_NEG;
		expr->unary.right = parse_prefix();
		return expr;
	} else if (match_token('!')) {
		Expression * expr = make_expr(EXPR_UNARY);
		expr->unary.type = OP_LNEG;
		expr->unary.right = parse_prefix();
		return expr;
	} else {
		return parse_postfix();
	}
}

Expression * parse_bool_ops()
{
	Expression * left = parse_prefix();
	while (is_token(TOKEN_EQ) || is_token(TOKEN_GTE) || is_token(TOKEN_LTE) || is_token('<') || is_token('>')) {
		Expression * expr = make_expr(EXPR_BINARY);
		expr->binary.type = token_to_bin_op[token.type];
		expr->binary.left = left;
		next_token();
		expr->binary.right = parse_prefix();
		left = expr;
	}
	return left;
}

Expression * parse_mul_ops()
{
	Expression * left = parse_bool_ops();
	while (is_token('*') || is_token('/') || is_token('%')) {
		Expression * expr = make_expr(EXPR_BINARY);
		expr->binary.type = token_to_bin_op[token.type];
		expr->binary.left = left;
		next_token();
		expr->binary.right = parse_bool_ops();
		left = expr;
	}
	return left;
}

Expression * parse_add_ops()
{
	Expression * left = parse_mul_ops();
	while (is_token('+') || is_token('-')) {
		Expression * expr = make_expr(EXPR_BINARY);
		expr->binary.type = token_to_bin_op[token.type];
		expr->binary.left = left;
		next_token();
		expr->binary.right = parse_mul_ops();
		left = expr;
	}
	return left;
}

Expression * parse_expression()
{
	return parse_add_ops();
}

Statement * parse_lone_expr()
{
	Statement * stmt = make_stmt(STMT_EXPR);
	stmt->stmt_expr.expr = parse_expression();
	expect_token(';');
	return stmt;
}

Statement * parse_assign()
{
	expect_token(TOKEN_SET);
	Statement * stmt = make_stmt(STMT_ASSIGN);
	stmt->stmt_assign.left = parse_expression();
	expect_token('=');
	stmt->stmt_assign.right = parse_expression();
	expect_token(';');
	return stmt;
}

Statement * parse_decl()
{
	expect_token(TOKEN_LET);
	Statement * stmt = make_stmt(STMT_DECL);
	check_token(TOKEN_NAME);
	stmt->stmt_decl.bind_name = token.name;
	next_token();
	expect_token('=');
	stmt->stmt_decl.bind_expr = parse_expression();
	expect_token(';');
	return stmt;
}

Statement * parse_if()
{
	expect_token(TOKEN_IF);
	Statement * stmt = make_stmt(STMT_IF);
	do {
		sb_push(stmt->stmt_if.conditions, parse_expression());
		sb_push(stmt->stmt_if.scopes,     parse_scope());
	} while (match_token(TOKEN_ELIF));
	if (match_token(TOKEN_ELSE)) {
		stmt->stmt_if.else_scope = parse_scope();
	}
	return stmt;
}

Statement * parse_while()
{
	expect_token(TOKEN_WHILE);
	Statement * stmt = make_stmt(STMT_WHILE);
	stmt->stmt_while.condition = parse_expression();
	stmt->stmt_while.scope = parse_scope();
	return stmt;
}

Statement * parse_scope()
{
	expect_token('{');
	Statement * stmt = make_stmt(STMT_SCOPE);
	while (!match_token('}')) {
		sb_push(stmt->stmt_scope.body, parse_statement());
	}
	return stmt;
}

Statement * parse_statement()
{
	switch (token.type) {
	case TOKEN_SET:
		return parse_assign();
		break;
	case TOKEN_LET:
		return parse_decl();
		break;
	case TOKEN_IF:
		return parse_if();
		break;
	case TOKEN_ELIF:
		fatal("elif outside of if chain");
		break;
	case TOKEN_ELSE:
		fatal("else outside of if chain");
		break;
	case TOKEN_WHILE:
		return parse_while();
		break;
	case '{':
		return parse_scope();
		break;
	default:
		return parse_lone_expr();
		break;
	}
}

Function * parse_function()
{
	Function * func = calloc(1, sizeof(Function));
	expect_token(TOKEN_FUNC);
	check_token(TOKEN_NAME);
	func->name = token.name;
	next_token();
	expect_token('(');
	Expression ** arglist = 0;
	if (!match_token(')')) {
		check_token(TOKEN_NAME);
		while (1) {
			sb_push(func->arg_names, token.name);
			next_token();
			if (!match_token(',')) {
				expect_token(')');
				break;
			}
		}
	}
	func->body = parse_scope();
}

void parse_test()
{
	//const char * source = "round(f() + 3, digits()) * -3;";
	//const char * source = "{let arr = new_arr(5); set arr[2] = 3;}";
	//const char * source = "if x == 2 { f(); } else { g(); }";
	const char * source =
		"func fib(count) {\n"
		"    let a = 0;\n"
		"    let b = 1;\n"
		"    let i = 0;\n"
		"    print(a);\n"
		"    while i < count {\n"
		"        set tmp = b;\n"
		"        set b = a + b;\n"
		"        set a = tmp;\n"
		"        print(b);\n"
		"        set i = i + 1;\n"
		"    }\n"
		"}\n";
	printf("%s\n\n", source);
	init_stream(source);
	while (!is_token(0)) {
		Function * func = parse_function();
		print_function(func);
		printf("\n");
	}
}

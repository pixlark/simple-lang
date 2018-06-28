#include "parser.h"

char * op_to_str[] = {
	[OP_NEG] = "NEG",
	[OP_ADD] = "+",
	[OP_SUB] = "-",
	[OP_MUL] = "*",
	[OP_DIV] = "/",
	[OP_EQ]  = "==",
	[OP_GT]  = ">",
	[OP_LT]  = "<",
	[OP_GTE] = ">=",
	[OP_LTE] = "<=",
};

Operator_Type token_to_bin_op[] = {
	['+']       = OP_ADD,
	['-']       = OP_SUB,
	['*']       = OP_MUL,
	['/']       = OP_DIV,
	[TOKEN_EQ]  = OP_EQ,
	['>']       = OP_GT,
	['<']       = OP_LT,
	[TOKEN_GTE] = OP_GTE,
	[TOKEN_LTE] = OP_GTE,
};

Statement * make_stmt(Stmt_Type type)
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
		break;
	}
	return stmt;
}

Expression * make_expr(Expr_Type type)
{
	Expression * expr = (Expression*) malloc(sizeof(Expression));
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

Expression * parse_expr_0()
{
	Expression * expr;
	if (token.type == '-') {
		expr = make_expr(EXPR_UNARY);
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
	return is_token('*') || is_token('/');
}

Expression * parse_expr_1()
{
	Expression * left = parse_expr_0();
	while (is_token_expr_1()) {
		Expression * expr = make_expr(EXPR_BINARY);
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
		Expression * expr = make_expr(EXPR_BINARY);
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
		Expression * expr = make_expr(EXPR_BINARY);
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
	Statement * stmt = make_stmt(STMT_LET);
	check_token(TOKEN_NAME);
	stmt->stmt_let.bind_name = token.name;
	next_token();
	expect_token('=');
	stmt->stmt_let.bind_val = parse_expression();
}

Statement * parse_while()
{
	Statement * stmt = make_stmt(STMT_WHILE);
	stmt->stmt_while.condition = parse_expression();
	expect_token('{');
	while (!match_token('}')) {
		sb_push(stmt->stmt_while.body, parse_statement());
	}
	return stmt;
}

Statement * parse_if()
{
	Statement * stmt = make_stmt(STMT_IF);
	stmt->stmt_if.condition = parse_expression();
	expect_token('{');
	while (!match_token('}')) {
		sb_push(stmt->stmt_if.body, parse_statement());
	}
	return stmt;
}

Statement * parse_print()
{
	Statement * stmt = make_stmt(STMT_PRINT);
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
		fatal("Not a statement");
	}
	expect_token(';');
	return stmt;
}

void parse_test()
{
	const char * source = \
		"if x {"
		"    let y = 1;"
		"    while y + 1 {"
		"        print 15;"
		"    };"
		"};";
	init_stream(source);
		
	Statement * if_s = parse_statement();
	assert(if_s->type == STMT_IF);
	assert(if_s->stmt_if.condition->type == EXPR_NAME);
	assert(if_s->stmt_if.condition->name.name == str_intern("x"));

	Statement * let_s = if_s->stmt_if.body[0];
	assert(let_s->type == STMT_LET);
	assert(let_s->stmt_let.bind_name == str_intern("y"));
	assert(let_s->stmt_let.bind_val->type == EXPR_LITERAL);
	assert(let_s->stmt_let.bind_val->literal.value == 1);

	Statement * while_s = if_s->stmt_if.body[1];
	assert(while_s->type == STMT_WHILE);
	assert(while_s->stmt_while.condition->type == EXPR_BINARY);
		
	Expression * bin = while_s->stmt_while.condition;
	assert(bin->binary.type == OP_ADD);
	assert(bin->binary.left->type == EXPR_NAME);
	assert(bin->binary.left->name.name == str_intern("y"));
	assert(bin->binary.right->type == EXPR_LITERAL);
	assert(bin->binary.right->literal.value == 1);

	Statement * print_s = while_s->stmt_while.body[0];
	assert(print_s->type == STMT_PRINT);
	assert(print_s->stmt_print.to_print->type == EXPR_LITERAL);
	assert(print_s->stmt_print.to_print->literal.value == 15);
}

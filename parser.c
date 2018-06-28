#include "parser.h"

char * op_to_str[10] = {
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

/* Operator precedences:
 *   HIGHEST
 * 0   - (negate)   | right-associative
 * 1   * /          | left-associative
 * 2   + -          | left-associative
 * 3   == > < >= <= | left-associative
 *   LOWEST
 */

Expression * parse_atom();
Expression * parse_expr_0();
Expression * parse_expr_1();
Expression * parse_expr_2();
Expression * parse_expression();
Statement  * parse_let();
Statement  * parse_while();
Statement  * parse_if();
Statement  * parse_statement();

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

char * load_string_from_file(char * path)
{
	FILE * file = fopen(path, "r");
	if (file == NULL) return NULL;
	int file_len = 0;
	while (fgetc(file) != EOF) file_len++;
	char * str = (char*) malloc(file_len + 1);
	str[file_len] = '\0';
	fseek(file, 0, SEEK_SET);
	for (int i = 0; i < file_len; i++) str[i] = fgetc(file);
	fclose(file);
	return str;
}

void parse_test()
{
	const char * source = load_string_from_file("fibonacci.sl");
	init_stream(source);
	while (token.type) {
		Statement * stmt = parse_statement();
		print_statement(stmt);
		printf("\n");
	}
}

void ast_test()
{
	// let x = 10;
	Statement * x_let = make_stmt(STMT_LET);
	x_let->stmt_let.bind_name = "x";

	Expression * x_let_val = make_expr(EXPR_LITERAL);
	x_let_val->literal.value = 10;
	
	x_let->stmt_let.bind_val = x_let_val;

	print_statement(x_let);
	printf("\n");

	// while x {
	//     let x = x - 1;
	// }
	Statement * x_while = make_stmt(STMT_WHILE);
	Expression * while_cond = make_expr(EXPR_NAME);
	while_cond->name.name = "x";
	x_while->stmt_while.condition = while_cond;

	Statement * x_dec = make_stmt(STMT_LET);
	x_dec->stmt_let.bind_name = "x";
	
	Expression * x_dec_val = make_expr(EXPR_BINARY);
	x_dec_val->binary.type = OP_SUB;
	Expression * x_dec_left = make_expr(EXPR_NAME);
	x_dec_left->name.name = "x";
	x_dec_val->binary.left = x_dec_left;
	Expression * x_dec_right = make_expr(EXPR_LITERAL);
	x_dec_right->literal.value = 1;
	x_dec_val->binary.right = x_dec_right;

	x_dec->stmt_let.bind_val = x_dec_val;

	sb_push(x_while->stmt_while.body, x_dec);

	print_statement(x_while);
	printf("\n");
}
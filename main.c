#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stretchy_buffer.h"

#define LEX_TEST_DEBUG       false
#define LEX_NEXT_TOKEN_DEBUG false
#define PARSE_INIT_DEBUG     false

#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define s8  int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

void fatal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	printf("Fatal Error:\n");
	vprintf(fmt, args);
	printf("\n");

	va_end(args);
	exit(1);
}

/*
 * <String Interning>
 */

typedef struct Str_Intern {
	size_t len;
	const char * str;
} Str_Intern;

static Str_Intern * str_interns;

const char * str_intern_range(const char * start, const char * end)
{
	size_t len = end - start;
	for (int i = 0; i < sb_count(str_interns); i++) {
		if (str_interns[i].len == len &&
			strncmp(str_interns[i].str, start, len) == 0) {
			return str_interns[i].str;
		}
	}
	char * interned = malloc(len + 1);
	strncpy(interned, start, len);
	interned[len] = '\0';
	Str_Intern new_intern = {len, interned};
	sb_push(str_interns, new_intern);
	return new_intern.str;
}

const char * str_intern(const char * str)
{
	return str_intern_range(str, str + strlen(str));
}

void str_intern_test()
{
	const char a[] = "asdf";
	const char b[] = "asdf";
	assert(a != b);
	assert(str_intern(a) == str_intern(b));
	const char c[] = "ASDF";
	assert(str_intern(a) != str_intern(c));
}

/* </String Interning> */

/*
 * <Lexer>
 */

typedef enum Token_Type {
	// Reserve first 128 values for ASCII terminals
	TOKEN_LITERAL = 128,
	TOKEN_NAME,
	TOKEN_LET,
	TOKEN_WHILE,
	TOKEN_IF,
	// Two-char nonterminals
	TOKEN_EQ,
	TOKEN_GTE,
	TOKEN_LTE,
} Token_Type;

typedef struct Token {
	Token_Type type;
	const char * source_start;
	const char * source_end;
	union {
		int literal;
		const char * name;
	};
} Token;

void token_type_str(char * buf, Token_Type type)
{
	if (type >= 0 && type < 128) {
		sprintf(buf, "'%c' (%d)", type != '\0' ? type : 'N', type);
	} else {
		switch (type) {
		case TOKEN_LITERAL:
			sprintf(buf, "Literal");
			break;
		case TOKEN_NAME:
			sprintf(buf, "Name");
			break;
		case TOKEN_LET:
			sprintf(buf, "Let");
			break;
		case TOKEN_WHILE:
			sprintf(buf, "While");
			break;
		case TOKEN_IF:
			sprintf(buf, "If");
			break;
		case TOKEN_GTE:
			sprintf(buf, ">=");
			break;
		case TOKEN_LTE:
			sprintf(buf, "<=");
			break;
		case TOKEN_EQ:
			sprintf(buf, "==");
			break;
		}
	}
}

void print_token(Token token)
{
	char token_str[256];
	token_type_str(token_str, token.type);
	if (token.type == TOKEN_LITERAL) {
		printf("%s: %d\n", token_str, token.literal);
	} else if (token.type == TOKEN_NAME) {
		printf("%s: \"%s\" (@%p)\n",
			token_str, token.name, token.name);
	} else {
		printf("%s\n", token_str);
	}
}

Token token;
const char * stream;

const char * let_keyword;
const char * while_keyword;
const char * if_keyword;

void next_token();

void lex_init()
{
	let_keyword = str_intern("let");
	while_keyword = str_intern("while");
	if_keyword = str_intern("if");
	#if PARSE_INIT_DEBUG
	printf("let:   %p\n", let_keyword);
	printf("while: %p\n", while_keyword);
	printf("if:    %p\n", if_keyword);
	#endif
}

void init_stream(const char * source)
{
	stream = source;
	next_token();
}

void _next_token()
{
	token.source_start = stream;
	if (isdigit(*stream)) {
		token.type = TOKEN_LITERAL;
		int val = 0;
		while (isdigit(*stream)) {
			val *= 10;
			val += *stream - '0';
			stream++;
		}
		token.literal = val;
	} else if (isalpha(*stream) || *stream == '_') {
		token.type = TOKEN_NAME;
		while (isalpha(*stream) || isdigit(*stream) || *stream == '_') {
			stream++;
		}
		token.name = str_intern_range(token.source_start, stream);
		// TODO(pixlark): Refactor this into pointer->enum hash table?
		if (token.name == let_keyword) {
			token.type = TOKEN_LET;
		} else if (token.name == while_keyword) {
			token.type = TOKEN_WHILE;
		} else if (token.name == if_keyword) {
			token.type = TOKEN_IF;
		}
	} else {
		switch (*stream) {
		case ' ':
		case '\t':
		case '\n':
			stream++;
			_next_token();
			break;
		case '>':
			stream++;
			if (*stream == '=') {
				stream++;
				token.type = TOKEN_GTE;
			} else {
				token.type = '>';
			}
			break;
		case '<':
			stream++;
			if (*stream == '=') {
				stream++;
				token.type = TOKEN_GTE;
			} else {
				token.type = '<';
			}
			break;
		case '=':
			stream++;
			if (*stream == '=') {
				stream++;
				token.type = TOKEN_EQ;
			} else {
				token.type = '=';
			}
			break;
		default:
			token.type = *stream++;
			break;
		}
	}
	token.source_end = stream;
}

void next_token()
{
	_next_token();
	#if LEX_NEXT_TOKEN_DEBUG
	print_token(token);
	#endif
}

bool is_token(Token_Type type)
{
	return token.type == type;
}

bool is_token_name(const char * name)
{
	return token.type == TOKEN_NAME && token.name == name;
}

/* Checks that the next token is of the expected type. If so, it
 * advances the stream.
 */ 
bool match_token(Token_Type type)
{
	if (is_token(type)) {
		next_token();
		return true;
	}
	return false;
}

void fatal_expected(Token_Type expected_type, Token_Type got_type)
{
	char expected[256];
	char got     [256];
	token_type_str(expected, expected_type);
	token_type_str(got,      got_type);
	fatal("Expected token %s, got token %s", expected, got);
}

/* Checks that the next token is of the expected type. If it's not,
 * an error occurs.
 */
bool expect_token(Token_Type type)
{
	if (is_token(type)) {
		next_token();
		return true;
	}
	fatal_expected(type, token.type);
}

bool check_token(Token_Type type) {
	if (is_token(type)) return true;
	fatal_expected(type, token.type);
}

#define _assert_token(x) \
	assert(match_token(x))
#define _assert_token_name(x) \
	assert(token.name == str_intern(x) && match_token(TOKEN_NAME))
#define _assert_token_literal(x) \
	assert(token.literal == (x) && match_token(TOKEN_LITERAL))
#define _assert_token_eof(x) \
	assert(is_token(EOF))

#if LEX_TEST_DEBUG
#define assert_token(x) (print_token(token), _assert_token(x))
#define assert_token_name(x) (print_token(token), _assert_token_name(x))
#define assert_token_literal(x) (print_token(token), _assert_token_literal(x))
#define assert_token_eof(x) (print_token(token), _assert_token_eof(x))
#else
#define assert_token _assert_token
#define assert_token_name _assert_token_name
#define assert_token_literal _assert_token_literal
#define assert_token_eof _assert_token_eof
#endif

void lex_test()
{
	const char * source =
		"let x = 15;\n"
		"while x >= 0 {\n"
		"    let x = x - 1;\n"
		"};";
	init_stream(source);
	assert_token(TOKEN_LET);
	assert_token_name("x");
	assert_token('=');
	assert_token_literal(15);
	assert_token(';');
	assert_token(TOKEN_WHILE);
	assert_token_name("x");
	assert_token(TOKEN_GTE);
	assert_token_literal(0);
	assert_token('{');
	assert_token(TOKEN_LET);
	assert_token_name("x");
	assert_token('=');
	assert_token_name("x");
	assert_token('-');
	assert_token_literal(1);
	assert_token(';');
	assert_token('}');
	assert_token(';');
}

/* </Lexer> */

/*
 * <Parser>
 */

typedef struct Stmt_Let   Stmt_Let;
typedef struct Stmt_While Stmt_While;
typedef struct Stmt_If    Stmt_If;
typedef struct Statement  Statement;
typedef struct Expression Expression;

typedef enum Stmt_Type {
	STMT_LET,
	STMT_WHILE,
	STMT_IF,
} Stmt_Type;

typedef enum Expr_Type {
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_NAME,
	EXPR_LITERAL,
} Expr_Type;

typedef enum Operator_Type {
	// Unary
	OP_NEG = 0,
	// Binary
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_EQ,
	OP_GT,
	OP_LT,
	OP_GTE,
	OP_LTE,
} Operator_Type;

char * op_to_str[10] = {
	[OP_NEG] = "-",
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

typedef struct Statement {
    Stmt_Type type;
	union {
		struct {
			const char * bind_name;
			Expression * bind_val;
		} stmt_let;
		struct {
			Expression * condition;
			Statement ** body;
		} stmt_while;
		struct {
			Expression * condition;
			Statement ** body;
		} stmt_if;
    };
} Statement;

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

typedef struct Expression {
    Expr_Type type;
	union {
		struct {
			Operator_Type type;
			Expression * right;
		} unary;
		struct {
			Operator_Type type;
			Expression * left;
			Expression * right;
		} binary;
		struct {
			const char * name;
		} name;
		struct {
			u64 value;
		} literal;
	};
} Expression;

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
		printf("%d", expr->literal.value);
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
		//Expression * expr = parse_expression();
		//print_expression(expr);
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

/* </Parser> */

int main()
{
	str_intern_test();
	lex_init();
	lex_test();
	parse_test();
	//ast_test();
	return 0;
}

#pragma once
#include "lexer.h"
#include "error.h"

typedef enum Stmt_Type {
	STMT_LET,
	STMT_WHILE,
	STMT_IF,
	STMT_PRINT,
} Stmt_Type;

typedef enum Expr_Type {
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_INDEX,
	EXPR_FUNCALL,
	EXPR_NAME,
	EXPR_LITERAL,
} Expr_Type;

typedef enum Operator_Type {
	// Unary
	OP_NEG = 0,
	OP_LNEG,
	// Binary
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
	OP_EQ,
	OP_GT,
	OP_LT,
	OP_GTE,
	OP_LTE,
} Operator_Type;

extern char * op_to_str[];
extern Operator_Type token_to_bin_op[];

typedef struct Statement  Statement;
typedef struct Expression Expression;

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
			bool has_else;
			Statement ** else_body;
		} stmt_if;
		struct {
			Expression * to_print;
		} stmt_print;
    };
	u32 line;
} Statement;

typedef struct Arg_List {
	Expression ** args;
	u32 arg_count;
} Arg_List;

typedef struct Expression {
    Expr_Type type;
	union {
		struct {
			Operator_Type type;
			Expression * right;
		} unary;
		struct {
			Expression * left;
			Expression * right;
		} index;
		struct {
			Expression * name;
			Expression ** args;
		} funcall;
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
	u32 line;
} Expression;

Statement * make_stmt(Stmt_Type type);
Expression * make_expr(Expr_Type type);

void print_statement(Statement * stmt);
void print_expression(Expression * expr);

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
Statement  * parse_print();
Statement  * parse_statement();

void parse_test();

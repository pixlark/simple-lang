#pragma once
#include "lexer.h"
#include "error.h"

typedef enum Stmt_Type {
	STMT_EXPR,
	STMT_ASSIGN,
	STMT_DECL,
	STMT_IF,
	STMT_WHILE,
	STMT_RETURN,
	STMT_SCOPE,
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

#include "compiler.h" // TODO(pixlark): fuck it
// From compiler.h
typedef struct Declaration Declaration;
//

typedef struct Statement  Statement;
typedef struct Expression Expression;
typedef struct Function   Function;

typedef struct Function {
	const char * name;
	const char ** arg_names;
	Statement * body;
	Declaration * decls;
	u64 ip_start;
} Function;

typedef struct Statement {
	Stmt_Type type;
	union {
		struct {
			Expression * expr;
		} stmt_expr;
		struct {
			Expression * left;
			Expression * right;
		} stmt_assign;
		struct {
			const char * name;
			//Expression * bind_expr;
		} stmt_decl;
		struct {
			Expression ** conditions;
			Statement ** scopes;
			Statement * else_scope;
		} stmt_if;
		struct {
			Expression * condition;
			Statement * scope;
		} stmt_while;
		struct {
			Expression * expr;
		} stmt_return;
		struct {
			Statement ** body;
		} stmt_scope;
    };
	u32 line;
} Statement;

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
			int decl_pos;
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
Expression * parse_postfix();
Expression * parse_prefix();
Expression * parse_bool_ops();
Expression * parse_mul_ops();
Expression * parse_add_ops();
Expression * parse_expression();
Statement  * parse_return();
Statement  * parse_while();
Statement  * parse_lone_expr();
Statement  * parse_assign();
Statement  * parse_decl();
Statement  * parse_if();

Statement * parse_scope();
Statement * parse_statement();

Function * parse_function();

void parse_test();

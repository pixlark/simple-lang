#pragma once

#include "common.h"
#include "intern.h"
#include "error.h"

typedef enum Token_Type {
	// Reserve first 128 values for ASCII terminals
	TOKEN_LITERAL = 128,
	TOKEN_NAME,
	TOKEN_LET,
	TOKEN_WHILE,
	TOKEN_IF,
	TOKEN_PRINT,
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

void token_type_str(char * buf, Token_Type type);

void print_token(Token token);

extern Token token;
extern const char * stream;

extern const char * let_keyword;
extern const char * while_keyword;
extern const char * if_keyword;

void lex_init();

void init_stream(const char * source);

void next_token();

bool is_token(Token_Type type);

bool is_token_name(const char * name);

/* Checks that the next token is of the expected type. If so, it
 * advances the stream.
 */ 
bool match_token(Token_Type type);

void fatal_expected(Token_Type expected_type, Token_Type got_type);

/* Checks that the next token is of the expected type. If it's not,
 * an error occurs.
 */
bool expect_token(Token_Type type);

bool check_token(Token_Type type);

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

void lex_test();

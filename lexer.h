#pragma once

#include "common.h"
#include "intern.h"
#include "error.h"
#include "map.h"

typedef enum Token_Type {
	// Reserve first 128 values for ASCII terminals
	TOKEN_LITERAL = 128,
	TOKEN_NAME,
	TOKEN_LET,
	TOKEN_SET,
	TOKEN_WHILE,
	TOKEN_IF,
	TOKEN_ELIF,
	TOKEN_ELSE,
	TOKEN_FUNC,
	TOKEN_RETURN,
	// Two-char nonterminals
	TOKEN_EQ,
	TOKEN_GTE,
	TOKEN_LTE,
} Token_Type;

typedef struct Token {
	Token_Type type;
	u32 line;
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
extern u32 current_line;
extern const char * stream;

void lex_init();
void init_stream(const char * source);

void next_token();
bool is_token(Token_Type type);
bool is_token_name(const char * name);

/* Checks that the next token is of the expected type. If so, it
 * advances the stream.
 */ 
bool match_token(Token_Type type);

/* Checks that the next token is of the expected type. If it's not,
 * an error occurs.
 */
bool expect_token(Token_Type type);
bool check_token(Token_Type type);

void fatal_expected(Token_Type expected_type, Token got_token);

void lex_test();

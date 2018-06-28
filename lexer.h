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

void print_token(Token token);

extern Token token;
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

void fatal_expected(Token_Type expected_type, Token_Type got_type);

void lex_test();

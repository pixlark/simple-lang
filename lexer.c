#include "lexer.h"

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

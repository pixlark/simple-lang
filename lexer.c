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
			sprintf(buf, "let");
			break;
		case TOKEN_WHILE:
			sprintf(buf, "while");
			break;
		case TOKEN_IF:
			sprintf(buf, "if");
			break;
		case TOKEN_ELIF:
			sprintf(buf, "elif");
			break;
		case TOKEN_ELSE:
			sprintf(buf, "else");
			break;
		case TOKEN_FUNC:
			sprintf(buf, "func");
			break;
		case TOKEN_RETURN:
			sprintf(buf, "return");
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
u32 current_line = 0;
const char * stream;

Map * keyword_map;

void lex_init()
{
	keyword_map = make_map(16);

	map_insert(keyword_map, (u64) str_intern("let"),     (u64) TOKEN_LET);
	map_insert(keyword_map, (u64) str_intern("set"),     (u64) TOKEN_SET);
	map_insert(keyword_map, (u64) str_intern("while"),   (u64) TOKEN_WHILE);
	map_insert(keyword_map, (u64) str_intern("if"),      (u64) TOKEN_IF);
	map_insert(keyword_map, (u64) str_intern("elif"),    (u64) TOKEN_ELIF);
	map_insert(keyword_map, (u64) str_intern("else"),    (u64) TOKEN_ELSE);
	map_insert(keyword_map, (u64) str_intern("func"),    (u64) TOKEN_FUNC);
	map_insert(keyword_map, (u64) str_intern("return"),  (u64) TOKEN_RETURN);
}

void init_stream(const char * source)
{
	stream = source;
	current_line = 0;
	next_token();
}

void _next_token()
{
	token.line = current_line;
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
		if (map_index(keyword_map, (u64) token.name, NULL)) {
			map_index(keyword_map, (u64) token.name, (u64*) &token.type);
		}
	} else {
		switch (*stream) {
		case ' ':
		case '\t':
		case '\n':
			if (*stream == '\n') {
				current_line++;
			}
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
				token.type = TOKEN_LTE;
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

void fatal_expected(Token_Type expected_type, Token got_token)
{
	char expected[256];
	char got     [256];
	token_type_str(expected, expected_type);
	token_type_str(got,      got_token.type);
	fatal_line(got_token.line, "Expected token %s, got token %s", expected, got);
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
	fatal_expected(type, token);
}

bool check_token(Token_Type type) {
	if (is_token(type)) return true;
	fatal_expected(type, token);
}

#define _assert_token(x) \
	assert(match_token(x))
#define _assert_token_name(x) \
	assert(token.name == str_intern(x) && match_token(TOKEN_NAME))
#define _assert_token_literal(x) \
	assert(token.literal == (x) && match_token(TOKEN_LITERAL))
#define _assert_token_eof(x) \
	assert(is_token('\0'))

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
	const char * source = "round(f() + 3, digits()) * -3";
	init_stream(source);
	assert_token_name("round");
	assert_token('(');
	assert_token_name("f");
	assert_token('(');
	assert_token(')');
	assert_token('+');
	assert_token_literal(3);
	assert_token(',');
	assert_token_name("digits");
	assert_token('(');
	assert_token(')');
	assert_token(')');
	assert_token('*');
	assert_token('-');
	assert_token_literal(3);
	assert_token_eof();
	return;
}

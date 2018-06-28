#include "common.h"
#include "error.h"
#include "intern.h"
#include "lexer.h"
#include "map.h"
#include "parser.h"
#include "vm.h"

int main()
{
	str_intern_test();
	map_test();
	lex_init();
	lex_test();
	//parse_test();
	//ast_test();
	vm_test();
	return 0;
}

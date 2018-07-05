#include "common.h"
#include "compiler.h"
#include "error.h"
#include "intern.h"
#include "lexer.h"
#include "map.h"
#include "parser.h"
#include "vm.h"

int main(int argc, char ** argv)
{
	lex_init();
	
	str_intern_test();
	map_test();
	
	lex_test();
	//parse_test();
	vm_test();

	if (argc < 2) {
		printf("Need a file to interpret.\n");
		return 1;
	} else if (argc > 2) {
		printf("Provide one file to interpret.\n");
		return 1;
	}
	
	const char * source = load_string_from_file(argv[1]);
	init_stream(source);

	Map * function_map = make_map(512);
	
	while (token.type) {
		Function * func = parse_function();
		compile_function(func);
		map_insert(function_map, (u64) func->name, (u64) func);
	}

	int iter = -1;
	while ((iter = map_iter(function_map, iter)) != -1) {
		printf("%s:\n", (char*) function_map->keys[iter]);
		Function * func = (Function*) function_map->values[iter];
		for (int i = 0; i < sb_count(func->decls); i++) {
			printf("  %s declared, size %zu\n", func->decls[i].name, func->decls[i].size);
		}
	}
}

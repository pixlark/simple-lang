#include "common.h"
//#include "compiler.h"
#include "error.h"
#include "intern.h"
#include "lexer.h"
#include "map.h"
#include "parser.h"
//#include "vm.h"

int main(int argc, char ** argv)
{
	lex_init();
	
	str_intern_test();
	map_test();
	
	lex_test();
	//parse_test();

	if (argc < 2) {
		printf("Need a file to interpret.\n");
		return 1;
	} else if (argc > 2) {
		printf("Provide one file to interpret.\n");
		return 1;
	}
	
	const char * source = load_string_from_file(argv[1]);
	init_stream(source);
	
	while (token.type) {
		Function * func = parse_function();
	}
	
	#if 0
	/*
	 * Initialization
	 */
	VM _vm;
	VM * vm = &_vm;
	vm_init(vm);
	lex_init();

	/*
	 * Testing
	 */
	str_intern_test();
	map_test();
	lex_test();
	parse_test();

	/*
	 * Run Interpreter
	 */
	if (argc < 2) {
		printf("Need a file to interpret.\n");
		return 1;
	} else if (argc > 2) {
		printf("Provide one file to interpret.\n");
		return 1;
	}
	
	const char * source = load_string_from_file(argv[1]);
	init_stream(source);
	
	#if DEBUG_PRINTING
	printf("-------\nSOURCE\n-------\n");
	printf("%s\n", source);
	printf("-------\nAST\n-------\n");
	#endif
	
	while (token.type) {
		Statement * stmt = parse_statement();
		compile_statement(vm, stmt);

		#if DEBUG_PRINTING
		print_statement(stmt);
		printf("\n");
		#endif
	}
	ipush(((Instruction){INST_HALT}), 0);

	#if DEBUG_PRINTING
	printf("-------\nINSTRUCTIONS\n-------\n");
	for (int i = 0; i < sb_count(vm->instructions); i++) {
		printf("%03d ", i);
		print_instruction(vm->instructions[i]);
	}
	#endif

	do {
		#if CPU_STATE_REPORTING
		vm_print_state(vm);
		#endif
	} while (vm_step(vm));

	#if DEBUG_PRINTING
	printf("-------\nEND VARIABLE STATE\n-------\n");
	for (int i = 0; i < 512; i++) {
		if (vm->symbol_table->taken[i]) {
			printf("%s = %ld\n", (char*) vm->symbol_table->keys[i], (s64) vm->symbol_table->values[i]);
		}
	}
	#endif
	
	return 0;
	#endif
}

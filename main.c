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

	#if 0
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
		prepare_function(func);
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
	#endif

	const char * source = "if 1 { 1 + 2; } elif 2 { 2 + 3; } else { 3 + 4; }";
	init_stream(source);

	Statement * stmt = parse_statement();
	
	VM _vm;
	VM * vm = &_vm;
	vm_init(vm);

	compile_statement(vm, stmt);
	EMIT(INST_HALT);

	for (int i = 0; i < sb_count(vm->insts); i++) {
		printf("%02d ", i);
		print_instruction(vm->insts[i]);
	}

	return 0;
	
	#define CYCLE_LIMIT 100
	int cycles = 0;
	do {
		printf("----\n");
		printf("IP: %d\n", vm->ip);
		printf("Call Stack (%lu):\n", vm->call_sp);
		if (vm->call_sp == 0) printf(" - \n");
		for (int i = vm->call_sp - 1; i >= 0; i--) {
			printf(" %ld\n", vm->call_stack[i]);
		}
		printf("Op Stack (%lu):\n", vm->op_sp);
		if (vm->op_sp == 0) printf(" - \n");
		for (int i = vm->op_sp - 1; i >= 0; i--) {
			printf(" %ld\n", vm->op_stack[i]);
		}
		printf("%s\n", inst_type_to_str[vm->insts[vm->ip].type]);
		cycles++;
		if (cycles >= CYCLE_LIMIT)
			internal_error("Cycle overflow");
	} while (vm_step(vm));
	printf("--------\n");
}

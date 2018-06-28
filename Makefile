make:
	gcc -g \
		main.c error.c intern.c common.c map.c stretchy_buffer.c \
		lexer.c parser.c vm.c compiler.c \
		-std=c99 \
		-o comp

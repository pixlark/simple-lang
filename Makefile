make:
	gcc -g \
		main.c error.c intern.c common.c map.c stretchy_buffer.c \
		lexer.c parser.c compiler.c vm.c \
		-std=c99 \
		-o comp

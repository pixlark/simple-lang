make:
	gcc -g main.c error.c intern.c lexer.c parser.c vm.c stretchy_buffer.c map.c \
		-std=c99 \
		-o comp

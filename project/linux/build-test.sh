DIR=$(pwd)
cd ../../src/clamc
gcc -g -L. -o $DIR/clamc-test \
	analyzer.c \
	ast.c \
	executor.c \
	generator.c \
	lexer.c \
	message.c \
	module.c \
	parser.c \
	source.c \
	stack.c \
	strings.c \
	token.c \
	vector.c \
	test.c
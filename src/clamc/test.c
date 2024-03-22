#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "source.h"
#include "lexer.h"
#include "parser.h"
#include "executor.h"
#include "analyzer.h"
#include "printer.h"

#include "vector.h"
#include "stack.h"


struct TestCase
{
	void(*base)(const char*);
	const char* baseName;
	const char* name;
	const char* code;
	bool wrong;
};
typedef struct TestCase TestCase;

static void TestCase_run(TestCase* t)
{
	printf("testing %s-%s\n", t->baseName, t->name);

	t->base(t->code);

	if (t->wrong)
		printf("test wrong %s-%s should not go here!!!\n", t->baseName, t->name);
	else
		printf("test %s-%s done.\n", t->baseName, t->name);
}

static void test_vector(const char* arg)
{
	Vector vec;
	Vector_init(&vec, sizeof(int));
	int i1 = 1;
	int i2 = 2;
	int i3 = 3;
	Vector_add(&vec, &i1);
	Vector_add(&vec, &i2);
	Vector_add(&vec, &i3);

	for (int i = 0; i < vec.size; i++)
		printf("vec[%d]=%d\n", i, *(int*)Vector_get(&vec, i));
}

static void test_stack(const char* arg)
{
	Stack stack;
	Stack_init(&stack, sizeof(int));

	int i1 = 1;
	int i2 = 2;
	int i3 = 3;
	Stack_push(&stack, &i1);
	Stack_push(&stack, &i2);
	Stack_push(&stack, &i3);

	int size = stack.size;
	while (size--)
		printf("stack.pop=%d\n", *(int*)Stack_pop(&stack));
}

static void test_string(const char* arg)
{
	String s = String_literal("hello");
	printf("s=" String_FMT, String_arg(s));
}

static void test_string_buffer(const char* arg)
{
	StringBuffer buf;
	StringBuffer_init(&buf);

	StringBuffer_append(&buf, "hello");
	StringBuffer_append(&buf, " world!");

	printf("buf=" String_FMT "\n", String_arg(buf));

	StringBuffer_reset(&buf);
	StringBuffer_append(&buf, "good");

	String s = String_literal(" morning.");
	StringBuffer_appendString(&buf, &s);

	printf("buf=" String_FMT "\n", String_arg(buf));

	StringBuffer s1;
	StringBuffer_init(&s1);

	StringBuffer_append(&s1, "clam, ");
	StringBuffer_appendString(&s1, StringBuffer_string(&buf));

	printf("s1=" String_FMT "\n", String_arg(s1));

	StringBuffer s2 = StringBuffer_clone(&s1);
	printf("s2=" String_FMT "\n", String_arg(s2));

	printf("s1.data=%p s2.data=%p\n", s1.data, s2.data);
}


static void test_lexer(const char* code)
{
	printf("source code:\n%s\n\n", code);

	Source src;
	Source_init(&src, code);

	Lexer lex;
	Lexer_init(&lex, &src);

	Printer p;
	Printer_init(&p);

	printf("token dump:\n");
	Printer_printLex(&p, &lex);
}

static void test_parser(const char* code)
{
	printf("source code:\n%s\n\n", code);

	Source source;
	Source_init(&source, code);

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Printer p;
	Printer_init(&p);

	printf("AST dump:\n");
	Printer_printAst(&p, module);
}

static void test_analyzer(const char* code)
{
	printf("source code:\n%s\n\n", code);

	Source source;
	Source_init(&source, code);

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);
	Analyzer_analyze(&anly, module);

	printf("sematic analysis done.\n");
}

static void test_executor(const char* code)
{
	printf("source code:\n%s\n\n", code);

	Source source;
	Source_init(&source, code);

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);
	Analyzer_analyze(&anly, module);

	printf("interpret output:\n");

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

static void test_generator(const char* code)
{
	printf("source code:\n%s\n\n", code);

	Source source;
	Source_init(&source, code);

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);
	Analyzer_analyze(&anly, module);

	Generator gen;
	Generator_init(&gen, GENERATE_TARGE_C);
	Generator_generate(&gen, module);

	StringBuffer buf;
	StringBuffer_init(&buf);

	Generator_getSource(&gen, &buf);

	printf("generate C header:\n" String_FMT "\n\n", String_arg(gen.header));
	printf("generate C source:\n" String_FMT "\n\n", String_arg(buf));
}

#define TEST(base, name, code)       { base, #base, name, code, false },
#define TEST_WRONG(base, name, code) { base, #base, name, code, true  },

static TestCase tests[] =
{
	TEST(test_vector, "basic", "")
	TEST(test_stack,  "basic", "")
	TEST(test_string, "basic", "")
	TEST(test_string_buffer, "basic", "")

	TEST(test_lexer, "eof"    , "")
	TEST(test_lexer, "keyword", "export void int return")
	TEST(test_lexer, "punct"  , "(){},;=")

	TEST(test_parser, "basic",                "export int main() { return 0; }")
	TEST(test_parser, "return_int",           "export int main() { return 666; }")
	TEST(test_parser, "global_variant1",      "int a; int b = 10; export int c = 666; export int d;")
	TEST(test_parser, "global_variant2",      "int _a = 1; int a2 = 2; export int a_3 = 3; export int __c = 4; export int _d1_ = 5;")
	TEST(test_parser, "void_function",        "void print() {}")
	TEST(test_parser, "functions",            "int foo() {} int bar(){} void print(){} export int main() { return 0; }")
	TEST(test_parser, "return_int",           "export int main() { return 666; }")
	TEST(test_parser, "function_parameter1",  "void print(int a, int b) { }")
	TEST(test_parser, "function_parameter2",  "void print(int a,  int b,) { }")
	TEST(test_parser, "function_argument1",   "void print(int a, int b) { } export int main() { print(1, 2); return 0; }")
	TEST(test_parser, "function_argument2",   "void print(int a, int b) { } export int main() { print(1, 2, 3,); return 0; }")
	TEST(test_parser, "functions_return_int", "int foo(){return 1;}\nint bar(){return 2;}\nexport int main() { return 666; }")
	TEST(test_parser, "multi_block1",          "export int main() { int a = 1; { int b = 2; } return a; }")
	TEST(test_parser, "multi_block2",          "export int main() { int a = 1; { int b = 2; { int c = 3; { int d = a; } } } return a; }")

	TEST_WRONG(test_parser, "basic_wrong1",              "export int main() { 0 return; }")
	TEST_WRONG(test_parser, "basic_wrong2",              "int export main() {}")
	TEST_WRONG(test_parser, "basic_wrong3",              "export main() {}")
	TEST_WRONG(test_parser, "global_variant_wrong1",     "int 1a = 0;")
	TEST_WRONG(test_parser, "function_parameter_wrong1", "void print(int a; int b) { }")
	TEST_WRONG(test_parser, "function_argument_wrong1",  "int foo(int a, int b) { return a; } export int main() { return foo(1 2); }")
	TEST_WRONG(test_parser, "function_argument_wrong2",  "int foo(int a, int b) { return a; } export int main() { return foo(1;2); }")

	TEST(test_analyzer,       "basic",               "export int main() { return 12345; }")
	TEST(test_analyzer,       "global_variant1",     "export int a;")
	TEST(test_analyzer,       "global_variant2",     "int foo() { return a; } int a = 0;")
	TEST(test_analyzer,       "local_variant1",      "export int main() { int a = 0; { int b = a; } return a; }")
	TEST(test_analyzer,       "function_parameter1", "int foo(int a, int b) { return a; }")
	TEST(test_analyzer,       "function_parameter2", "int foo(int a, int b, int c, int d) { return a; return b; return c; return d; }")
	TEST(test_analyzer,       "function_argument1",  "int foo(int a, int b) { return a; } export int main() { return foo(1, 2); }")
	TEST(test_analyzer,       "function_argument2",  "int foo(int a, int b) { return a; } int bar() { return 1; } export int main() { return foo(bar(), 2); }")

	TEST_WRONG(test_analyzer, "basic_wrong1",              "int main() { return 0; }")
	TEST_WRONG(test_analyzer, "basic_wrong2",              "export void main() { return 0; }")
	TEST_WRONG(test_analyzer, "basic_wrong3",              "export int main() { return foo(); }")
	TEST_WRONG(test_analyzer, "global_variant_wrong1",     "export int a = 0; int a = 1;")
	TEST_WRONG(test_analyzer, "global_variant_wrong2",     "void a;")
	TEST_WRONG(test_analyzer, "global_variant_wrong3",     "int a = \"1\";")
	TEST_WRONG(test_analyzer, "function_parameter_wrong1", "int foo(int a, int b, int c, int d) { int a = 0; return a; }")
	TEST_WRONG(test_analyzer, "function_argument_wrong1",  "int foo(int a, int b) { return a; } export int main() { foo(1, 2, 3); return 0; }")
	TEST_WRONG(test_analyzer, "function_argument_wrong2",  "int foo(int a, int b) { return a; } void print() {} export int main() { return foo(1, print()); }")

	TEST(test_executor, "basic",                "export int main() { return 12345; }")
	TEST(test_executor, "void_function",        "void foo() { } export int main() { foo(); return 0; }")
	TEST(test_executor, "function_call1",       "int foo() { return 1; } export int main() { return foo(); }")
	TEST(test_executor, "function_call2",       "int foo() { return 666; } int bar() { return foo(); } export int main() { return bar(); }")
	TEST(test_executor, "function_call3",       "export int main() { return bar(); } int bar() { return foo(); } int foo() { return 666; }")
	TEST(test_executor, "global_variant1",      "int a = 666; export int main() { return a; }")
	TEST(test_executor, "global_variant2",      "export int main() { return a; }\nint a = 1234;")
	TEST(test_executor, "global_variant_init1", "int a = foo(); int foo() { return 1234; } export int main() { return a; }")
	TEST(test_executor, "local_variant1",       "export int main() { int a = 888; return a; }")
	TEST(test_executor, "local_variant2",       "export int main() { int a = 666; { return a; } }")
	TEST(test_executor, "local_variant3",       "export int main() { int a = 123; { int b = a; return b; } }")
	TEST(test_executor, "local_variant4",       "export int main() { int a = 1; { int a = 2; return a; } }")
	TEST(test_executor, "multi_block1",          "export int main() { int a = 1; { int a = 2; { int b = a; return b; } } }")
	TEST(test_executor, "multi_block2",          "export int main() { int a = 1; { int b = 2; } int b = 3; { int a = 4; int b = 5;} return b; }")
	TEST(test_executor, "function_argument1",   "int foo(int a, int b) { return a; } export int main() { return foo(1, 2); }")
	TEST(test_executor, "function_argument2",   "int foo(int a, int b) { return a; } int bar() { return 6; } export int main() { return foo(bar(), 2); }")
	TEST(test_executor, "function_argument3",   "int f1(int a) { return a; } int f2(int b) { return b; } int f3(int c) { return c; } export int main() { return f1(f2(f3(4))); }")

	TEST_WRONG(test_executor, "main_wrong1",           "int main() { return 12345; }")
	TEST_WRONG(test_executor, "main_wrong2",           "export void main() { return 0; }")
	TEST_WRONG(test_executor, "no_return_wrong",       "export int main() { return; }")
	TEST_WRONG(test_executor, "global_variant_wrong1", "int a = 0; export int main() { return b; }")
	TEST_WRONG(test_executor, "local_variant_wrong1",  "export int main() { { int a = 2; } return a; }")

	TEST(test_generator, "basic", "export int main() { return 0; }")
	TEST(test_generator, "function_call1", "int foo() { return 1; } export int main() { return foo(); }")
	TEST(test_generator, "function_call2", "export int main() { return test1(); } int test1() { return test2(); } int test2() { return 888; }")
	TEST(test_generator, "function_call3", "export int main() { return bar(); } int bar() { return foo(); } int foo() { return 666; }")
	TEST(test_generator, "global_variant1", "int a = 0; export int b = 1; export int main() { return b; }")
	TEST(test_generator, "global_variant2", "int a = foo(); int foo() { return 9527; } export int main() { return a; }")
	TEST(test_generator, "global_variant3", "export int main() { return a; } int a = 9697;")
	TEST(test_generator, "local_variant1", "export int main() { int a = 0; int b = a; return a; }")
	TEST(test_generator, "local_variant2", "export int main() { int a = 0; { int b = a; } int b = 1; return b; }")

	TEST_WRONG(test_generator, "local_variant_wrong1", "export int main() { int a = b; { int b = 0; } return 0; }")
	TEST_WRONG(test_generator, "local_variant_wrong2", "export int main() { int a = 0; { int b = a; } return b; }")
};
#undef TEST
#undef TEST_WRONG

#define TEST(base, name, code) \
	do { TestCase test = { base, #base, name, code, false }; TestCase_run(&test);  return 0; } while(0);
#define TEST_WRONG(base, name, code) \
	do { TestCase test = { base, #base, name, code, true  }; TestCase_run(&test);  return 0; } while(0);

static void usage()
{
	printf(
		"usage: clamc-test [options]\n"
		"-l\tshow test case list\n"
		"-r <id>\trun specified test case\n"
		"-t <base> <file>\t test code file\n"
		"-h\tshow usage\n"
		//"-v\tshow version\n"
		"\n"
		"if no options, all built-in test cases.\n"
	);
}

int main(int argc, char** argv)
{

	//options
	bool all = true;
	bool run = false;
	int id = 0;
	bool list = false;
	bool test = false;
	const char* base = NULL;
	const char* file = NULL;

	//parse
	int c = argc;
	char** v = argv;
	--c; ++v;
	while (c)
	{
		if (strcmp(*v, "-h") == 0)
		{
			usage();
			return 0;
		}
		else if (strcmp(*v, "-l") == 0)
		{
			all = false;
			list = true;
		}
		//else if (strcmp(*argv, "-v") == 0)
		//	version();
		else if (strcmp(*v, "-r") == 0)
		{
			all = false;
			run = true;

			--c; ++v;
			if (!c)
			{
				printf("no id provided!\n");
				return 0;
			}
			id = atoi(*v);
		}
		else if (strcmp(*v, "-t") == 0)
		{
			all = false;
			test = true;

			--c; ++v;
			if (!c)
			{
				printf("no base provided!\n");
				return 0;
			}
			base = *v;

			--c; ++v;
			if (!c)
			{
				printf("no file provided!\n");
				return 0;
			}
			file = *v;
		}
		else if ((*v)[0] == '-')
		{
			printf("unknown options '%s'\n", *v);
			return -1;
		}
		else
			break;

		--c; ++v;
	}

	const int count = sizeof(tests) / sizeof(tests[0]);

	if (list)
	{
		printf("ID\tBase\tName\n");
		for (int i = 0; i < count; ++i)
		{
			TestCase* test = &tests[i];
			printf("%d\t%s\t%s\n", i, test->baseName, test->name);
		}
	}

	if (run)
	{
		if (id < 0 || id >= count)
		{
			printf("test case id %d not found!\n", id);
			return 0;
		}

		TestCase_run(&tests[id]);
		return 0;
	}

	if (test)
	{
		//open file
		FILE* f = fopen(file, "rb");
		if (!f)
			return false;

		//get file size
		int beg = ftell(f);
		fseek(f, 0, SEEK_END);
		int end = ftell(f);
		int size = end - beg;
		fseek(f, beg, SEEK_SET);

		if (!size)
		{
			fclose(f);
			printf("empty file!\n");
			return 0;
		}

		char* buf = (char*)malloc(size + 1);  //TODO: mmap large file
		if (!buf)
		{
			fclose(f);
			printf("open file out of memory!\n");
			return 0;
		}
		buf[size] = 0;    //null terminal

		fread(buf, 1, size, f);
		fclose(f);

		if (strcmp(base, "lexer") == 0)
		{
			TEST(test_lexer, file, buf)
		}
		else if (strcmp(base, "parser") == 0)
		{
			TEST(test_parser, file, buf)
		}
		else if (strcmp(base, "analyzer") == 0)
		{
			TEST(test_analyzer, file, buf)
		}
		else if (strcmp(base, "executor") == 0)
		{
			TEST(test_executor, file, buf)
		}
		else if (strcmp(base, "generator") == 0)
		{
			TEST(test_generator, file, buf)
		}
		else
		{
			printf("unknown base %s!\n", base);
		}

		return 0;
	}

	if (!all)
		return 0;

	//run all
	char cmd[4096];
	int pos = sprintf(cmd, "%s -r ", argv[0]);

	for (int i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
	{
		int len = sprintf(cmd + pos, "%d", i);
		cmd[pos + len] = 0;
		//printf("cmd=%s\n", cmd);
		system(cmd);
		printf("\n\n");
	}

	return 0;
}
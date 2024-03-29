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
	TEST(test_lexer, "keyword", "export void int bool true false if else return")
	TEST(test_lexer, "punct"  , "(){},;")
	TEST(test_lexer, "operator", "=+-*/% ++ -- += -= *= /= %= ! != == < <= > >= &")

	TEST(test_parser, "basic",                "export int main() { return 0; }")
	TEST(test_parser, "functions1",           "void print() {} int foo() { return 0; } bool check() { return true; }")
	TEST(test_parser, "functions2",           "int a = 0; bool b = true; int foo() { return a; } bool check() { return b; }")
	TEST(test_parser, "function_parameter",   "int first(int a, int b) { return a; } bool enabled(bool b) { return b; }")
	TEST(test_parser, "function_argument",    "void first(int a, int b) { return a; } export int main() { first(1, 2); return 0; }")
	TEST(test_parser, "global_variant",       "int _a1 = 1; int a_2 = 2; export int a3_ = 3; bool b1; bool b2 = false; export bool b3 = true;")
	TEST(test_parser, "local_variant",        "export int main() { int a = 0; a = 1; int b = a; bool c = true; c = false; return a; }")
	TEST(test_parser, "multi_block",          "export int main() { int a = 1; { int b = 2; { int c = 3; { int d = a; } } } return a; }")
	TEST(test_parser, "add_expression",       "export int main() { int a = 1 + 2 + 3; int b = 4; a = a + b; return a + 5; }")
	TEST(test_parser, "sub_expression",       "int a = 1 - 1; int b = 1 - a; int c = 3 - 2 - 1 - a;")
	TEST(test_parser, "mul_expression",       "int a =  2 * 2; int b = a * 2; int c = a * 2 * b;")
	TEST(test_parser, "div_expression",       "int a =  10 / 2; int b = a / 2; int c = 10 / a / b;")
	TEST(test_parser, "mod_expression",       "int a =  18 % 4; int b = a % 2; int c = 10 % 3 % a;")
	TEST(test_parser, "plus_expression",      "export int main() { int a = 1; a = +1; int b = +a; return a; }")
	TEST(test_parser, "minus_expression",     "export int main() { int a = -1; int b = -a; return a; }")

	TEST(test_parser, "operation",            "int a = 1 * -2 + 3 * 4 - 6 / 2 + 19 % +2; int b = a + 3 * -4;")
	TEST(test_parser, "inc_statement1",       "int a = 0; void foo() { a++; }")
	TEST(test_parser, "dec_statement1",       "int a = 3; void foo() { a--; }")

	TEST(test_parser, "add_assign_statement", "int a = 0; void foo() { a += 1; int b = 0; b += a; }")
	TEST(test_parser, "sub_assign_statement", "int a = 2; void foo() { a -= 1; int b = 5; b -= a; }")
	TEST(test_parser, "mul_assign_statement", "int a = 2; void foo() { a *= 2; int b = 1; b *= a; }")
	TEST(test_parser, "div_assign_statement", "int a = 4; void foo() { a /= 2; int b = 10; b /= a; }")
	TEST(test_parser, "mod_assign_statement", "int a = 9; void foo() { a %= 2; int b = 6; b %= a; }")
	TEST(test_parser, "not_expression",       "bool a = !false; bool b = !a;")
	TEST(test_parser, "not_equal_expression", "bool a = 1 != 2; bool b = 1 + 1 != 2; bool c = 1 + 1 != 2 * 2;")
	TEST(test_parser, "equals_expression",    "bool a = 1 == 2; bool b = 1 + 1 == 2; bool c = 1 + 1 == 3 - 1; bool d = b == false;")
	TEST(test_parser, "less_expression",      "bool a = 1 < 2; bool b = 1 + 1 < 2; bool c = 1 + 1 < 3 - 1;")
	TEST(test_parser, "great_expression",     "bool a = 2 > 1; bool b = 2 + 1 > 2; bool c = 1 + 1 > 3 - 1;")
	TEST(test_parser, "le_expression",        "bool a = 2 <= 2; bool b = 1 + 1 <= 2; bool c = 1 + 1 <= 3 - 1;")
	TEST(test_parser, "ge_expression",        "bool a = 2 >= 2; bool b = 1 + 1 >= 2; bool c = 1 + 1 >= 3 - 1;")

	TEST(test_parser, "bitand_expression",    "int a = 3 & 1;")

	TEST(test_parser, "if_statement1",        "export int main() { if (true) return 1; return 0; }")
	TEST(test_parser, "if_statement2",        "export int main() { if (true) { return 1; } return 0; }")
	TEST(test_parser, "if_statement3",        "export int main() { if (!false) { return 1; } return 0; }")
	TEST(test_parser, "if_statement4",        "export int main() { if (true) return 1; else return 2; return 0; }")
	TEST(test_parser, "if_statement5",        "export int main() { if (!false) return 1; else { return 2; } return 0; }")
	TEST(test_parser, "if_statement6",        "export int main() { if (true) if (!false) if (true) return 1;  return 0; }")
	TEST(test_parser, "if_statement7",        "export int main() { if (true) if (!false) return 1; else return 2; else return 3;  return 0; }")
	TEST(test_parser, "if_statement8",        "export int main() { if (true) return 1; else if (!false) return 2; else return 3;  return 0; }")

	TEST_WRONG(test_parser, "basic_wrong1",              "export int main() { 0 return; }")
	TEST_WRONG(test_parser, "basic_wrong2",              "int export main() {}")
	TEST_WRONG(test_parser, "basic_wrong3",              "export main() {}")
	TEST_WRONG(test_parser, "variant_wrong",             "int 1a = 0;")
	TEST_WRONG(test_parser, "variant_assign_wrong",      "int foo() { int a = 1; int b = 2; a = b = 0; }")
	TEST_WRONG(test_parser, "function_parameter_wrong1", "void print(int a; int b) { }")
	TEST_WRONG(test_parser, "function_argument_wrong1",  "int foo(int a, int b) { return a; } export int main() { return foo(1 2); }")
	TEST_WRONG(test_parser, "function_argument_wrong2",  "int foo(int a, int b) { return a; } export int main() { return foo(1;2); }")
	TEST_WRONG(test_parser, "inc_statement_wrong1",      "int a = 1++;")
	TEST_WRONG(test_parser, "inc_statement_wrong2",      "int a = 0; int b = a++;")
	TEST_WRONG(test_parser, "dec_statement_wrong1",      "int a = 1--;")
	TEST_WRONG(test_parser, "dec_statement_wrong2",      "int a = 1; int b = a--;")
	TEST_WRONG(test_parser, "add_assign_statement_wrong1", "int a = 1; int b = a += 1;")
	TEST_WRONG(test_parser, "sub_assign_statement_wrong1", "int a = 1; int b = a -= 1;")
	TEST_WRONG(test_parser, "mul_assign_statement_wrong1", "int a = 2; int b = a *= 2;")
	TEST_WRONG(test_parser, "div_assign_statement_wrong1", "int a = 2; int b = a /= 2;")

	TEST(test_analyzer, "basic",                "export int main() { return 12345; }")
	TEST(test_analyzer, "global_variant",       "export int a; int foo() { return b; } int b = 0;")
	TEST(test_analyzer, "local_variant",        "export int main() { int a = 0; a = 1; int b = a; bool c = true; c = false; return a; }")
	TEST(test_analyzer, "function_parameter",   "int foo(int a, int b, int c, int d) { a; b; c; d; return a; }")
	TEST(test_analyzer, "function_argument",    "int foo(int a, int b) { return a; } int bar() { return 1; } export int main() { return foo(bar(), 2); }")
	TEST(test_analyzer, "variant_assign",       "int a = 0; bool b = false; void func() { a = 1; b = true; }")
	TEST(test_analyzer, "add_expression1",      "int a = 1 + 2 + 3; int b = a + 4;")
	TEST(test_analyzer, "add_expression2",      "int a = foo() + bar(); int foo() { return 1; } int bar() { return 2; }")
	TEST(test_analyzer, "plus_expression",      "export int main() { int a = 1; a = +1; int b = +a; return a; }")

	TEST(test_analyzer, "add_expression",       "export int main() { int a = 1 + 2 + 3; int b = 4; a = a + b; return a + 5; }")
	TEST(test_analyzer, "sub_expression",       "int a = 1 - 1; int b = 1 - a; int c = 3 - 2 - 1 - a;")
	TEST(test_analyzer, "mul_expression",       "int a =  2 * 2; int b = a * 2; int c = a * 2 * b;")
	TEST(test_analyzer, "div_expression",       "int a =  10 / 2; int b = a / 2; int c = 10 / a / b;")
	TEST(test_analyzer, "mod_expression",       "int a =  18 % 4; int b = a % 2; int c = 10 % 3 % a;")
	TEST(test_analyzer, "plus_expression",      "export int main() { int a = 1; a = +1; int b = +a; return a; }")
	TEST(test_analyzer, "minus_expression",     "export int main() { int a = -1; int b = -a; return a; }")
	TEST(test_analyzer, "operation",            "int a = 1 * -2 + 3 * 4 - 6 / 2 + 19 % +2; int b = a + 3 * -4;")
	TEST(test_analyzer, "inc_statement",        "int a = 0; void foo() { a++; }")
	TEST(test_analyzer, "dec_statement",        "int a = 3; void foo() { a--; }")

	TEST(test_analyzer, "add_assign_statement", "int a = 0; void foo() { a += 1; int b = 0; b += a; }")
	TEST(test_analyzer, "sub_assign_statement", "int a = 2; void foo() { a -= 1; int b = 5; b -= a; }")
	TEST(test_analyzer, "mul_assign_statement", "int a = 2; void foo() { a *= 2; int b = 1; b *= a; }")
	TEST(test_analyzer, "div_assign_statement", "int a = 4; void foo() { a /= 2; int b = 10; b /= a; }")
	TEST(test_analyzer, "mod_assign_statement", "int a = 9; void foo() { a %= 2; int b = 6; b %= a; }")
	TEST(test_analyzer, "not_expression",       "bool a = !false; bool b = !a;")
	TEST(test_analyzer, "not_equal_expression", "bool a = 1 != 2; bool b = 1 + 1 != 2; bool c = 1 + 1 != 2 * 2;")
	TEST(test_analyzer, "equals_expression",    "bool a = 1 == 2; bool b = 1 + 1 == 2; bool c = 1 + 1 == 3 - 1; bool d = b == false;")
	TEST(test_analyzer, "less_expression",      "bool a = 1 < 2; bool b = 1 + 1 < 2; bool c = 1 + 1 < 3 - 1;")
	TEST(test_analyzer, "great_expression",     "bool a = 2 > 1; bool b = 1 + 1 > 2; bool c = 3 - 1 > 2 - 1;")
	TEST(test_analyzer, "le_expression",        "bool a = 2 <= 2; bool b = 1 + 1 <= 2; bool c = 1 + 1 <= 3 - 1;")
	TEST(test_analyzer, "ge_expression",        "bool a = 2 >= 2; bool b = 1 + 1 >= 2; bool c = 1 + 1 >= 3 - 1;")

	TEST(test_analyzer, "if_statement1",        "export int main() { if (true) return 1; return 0; }")
	TEST(test_analyzer, "if_statement2",        "export int main() { if (true) { return 1; } return 0; }")
	TEST(test_analyzer, "if_statement3",        "export int main() { if (!false) { return 1; } return 0; }")
	TEST(test_analyzer, "if_statement4",        "export int main() { if (true) return 1; else return 2; return 0; }")
	TEST(test_analyzer, "if_statement5",        "export int main() { if (!false) return 1; else { return 2; } return 0; }")
	TEST(test_analyzer, "if_statement6",        "export int main() { if (true) if (!false) if (true) return 1;  return 0; }")
	TEST(test_analyzer, "if_statement7",        "export int main() { if (true) if (!false) return 1; else return 2; else return 3;  return 0; }")
	TEST(test_analyzer, "if_statement8",        "export int main() { if (true) return 1; else if (!false) return 2; else return 3;  return 0; }")
	TEST(test_analyzer, "if_statement9",        "export int main() { if (false) { return 1; } else return 2; }")
	TEST(test_analyzer, "if_statement10",       "export int main() { if (false) if (true) return 1; else return 2; }")  //TODO: flow analysis make it pass

	TEST(test_analyzer, "bitand_expression",    "int a = 3 & 1;")

	TEST_WRONG(test_analyzer, "basic_wrong1",                "int main() { return 0; }")
	TEST_WRONG(test_analyzer, "basic_wrong2",                "export void main() { return 0; }")
	TEST_WRONG(test_analyzer, "basic_wrong3",                "export int main() { return foo(); }")
	TEST_WRONG(test_analyzer, "global_variant_wrong1",       "export int a = 0; int a = 1;")
	TEST_WRONG(test_analyzer, "global_variant_wrong2",       "void a;")
	TEST_WRONG(test_analyzer, "global_variant_wrong3",       "int a = \"1\";")
	TEST_WRONG(test_analyzer, "function_parameter_wrong1",   "int foo(int a, int b, int c, int d) { int a = 0; return a; }")
	TEST_WRONG(test_analyzer, "function_argument_wrong1",    "int foo(int a, int b) { return a; } export int main() { foo(1, 2, 3); return 0; }")
	TEST_WRONG(test_analyzer, "function_argument_wrong2",    "int foo(int a, int b) { return a; } void print() {} export int main() { return foo(1, print()); }")
	TEST_WRONG(test_analyzer, "variant_assign_wrong1",       "void func() { 1 = 0; }")
	TEST_WRONG(test_analyzer, "variant_assign_wrong2",       "void foo() {} void func() { int a; a = foo(); }")
	TEST_WRONG(test_analyzer, "add_expression_wrong1",       "int a = 1 + foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "add_expression_wrong2",       "int a = 1 + b;")
	TEST_WRONG(test_analyzer, "plus_expression_wrong1",      "int a = +foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "plus_expression_wrong2",      "int a = +b;")
	TEST_WRONG(test_analyzer, "sub_expression_wrong1",       "int a = b - 1;")
	TEST_WRONG(test_analyzer, "sub_expression_wrong2",       "int a = 1 - foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "minus_expression_wrong1",     "int a = -b;")
	TEST_WRONG(test_analyzer, "minus_expression_wrong2",     "int a = -foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "mul_expression_wrong1",       "int a = 2 * b;")
	TEST_WRONG(test_analyzer, "mul_expression_wrong2",       "int a = 2 * foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "div_expression_wrong1",       "int a = 2 / b;")
	TEST_WRONG(test_analyzer, "div_expression_wrong2",       "int a = 2 / foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "div_expression_wrong3",       "int a = 2 / 0;")
	TEST_WRONG(test_analyzer, "mod_expression_wrong1",       "int a = 2 % b;")
	TEST_WRONG(test_analyzer, "mod_expression_wrong2",       "int a = 2 % foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "mod_expression_wrong3",       "int a = 2 % 0;")
	TEST_WRONG(test_analyzer, "operation_wrong1",            "int a = a - b + 1;")
	TEST_WRONG(test_analyzer, "operation_wrong2",            "int a = 1 + foo() - bar(); int foo() { return 0; } void bar() {}")
	TEST_WRONG(test_analyzer, "inc_statement_wrong1",        "void foo() { a++; }")
	TEST_WRONG(test_analyzer, "inc_statement_wrong2",        "void foo() { 1++; }")
	TEST_WRONG(test_analyzer, "inc_statement_wrong3",        "int bar() { return 1; } void foo() { bar()++; }")
	TEST_WRONG(test_analyzer, "dec_statement_wrong1",        "void foo() { a--; }")
	TEST_WRONG(test_analyzer, "dec_statement_wrong2",        "void foo() { 1--; }")
	TEST_WRONG(test_analyzer, "dec_statement_wrong3",        "int bar() { return 1; } void foo() { bar()--; }")
	TEST_WRONG(test_analyzer, "add_assign_statement_wrong1", "void foo() { 1 += 1; }")
	TEST_WRONG(test_analyzer, "add_assign_statement_wrong2", "void foo() { int a = 0; a += bar(); } void bar() {}")
	TEST_WRONG(test_analyzer, "sub_assign_statement_wrong1", "void foo() { 1 -= 1; }")
	TEST_WRONG(test_analyzer, "sub_assign_statement_wrong2", "void foo() { int a = 5; a -= bar(); } void bar() {}")
	TEST_WRONG(test_analyzer, "div_assign_statement_wrong1", "void foo() { 2 /= 1; }")
	TEST_WRONG(test_analyzer, "div_assign_statement_wrong2", "void foo() { int a = 1;  a /= bar(); } void bar() {}" )
	TEST_WRONG(test_analyzer, "div_assign_statement_wrong3", "void foo() { int a = 1;  a /= 0; }")
	TEST_WRONG(test_analyzer, "mod_assign_statement_wrong1", "void foo() { 2 %= 1; }")
	TEST_WRONG(test_analyzer, "mod_assign_statement_wrong2", "void foo() { int a = 1;  a %= bar(); } void bar() {}" )
	TEST_WRONG(test_analyzer, "mod_assign_statement_wrong3", "void foo() { int a = 1;  a %= 0; }")
	TEST_WRONG(test_analyzer, "not_expression_wrong1",       "bool a = 0;")
	TEST_WRONG(test_analyzer, "not_expression_wrong2",       "int a = 0; bool b = !a;")
	TEST_WRONG(test_analyzer, "not_equal_expression_wrong1", "int a = 0; bool b = a != true;")
	TEST_WRONG(test_analyzer, "not_equal_expression_wrong2", "int a = 0; bool b = a != foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "equals_expression_wrong1",    "int a = 0; bool b = a == true;")
	TEST_WRONG(test_analyzer, "equals_expression_wrong2",    "int a = 0; bool b = a == foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "less_expression_wrong1",      "int a = 0; bool b = a < foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "great_expression_wrong1",     "int a = 0; bool b = a > foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "le_expression_wrong1",        "int a = 0; bool b = a <= foo(); void foo() {}")
	TEST_WRONG(test_analyzer, "ge_expression_wrong1",        "int a = 0; bool b = a >= foo(); void foo() {}")

	TEST_WRONG(test_analyzer, "if_statement_wrong1",        "export int main() { if (0) return 1; return 0; }")
	TEST_WRONG(test_analyzer, "if_statement_wrong2",        "export int main() { if (1 + 1) { return 1; } return 0; }")
	TEST_WRONG(test_analyzer, "if_statement_wrong3",        "export int main() { if (false) { return 1; } }")

	TEST_WRONG(test_analyzer, "bitand_expression_wrong1",   "int a = true & 1;")
	TEST_WRONG(test_analyzer, "bitand_expression_wrong2",   "int a = b & 1;")

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
	TEST(test_executor, "multi_block1",         "export int main() { int a = 1; { int a = 2; { int b = a; return b; } } }")
	TEST(test_executor, "multi_block2",         "export int main() { int a = 1; { int b = 2; } int b = 3; { int a = 4; int b = 5;} return b; }")
	TEST(test_executor, "function_argument1",   "int foo(int a, int b) { return a; } export int main() { return foo(1, 2); }")
	TEST(test_executor, "function_argument2",   "int foo(int a, int b) { return a; } int bar() { return 6; } export int main() { return foo(bar(), 2); }")
	TEST(test_executor, "function_argument3",   "int f1(int a) { return a; } int f2(int b) { return b; } int f3(int c) { return c; } export int main() { return f1(f2(f3(4))); }")
	TEST(test_executor, "variant_assign1",      "int a = 0; export int main() { a = 1; return a; }")
	TEST(test_executor, "variant_assign2",      "export int main() { int a = 1; a = 2; return a; }")
	TEST(test_executor, "variant_assign3",      "export int main() { int a = 2; return a = 3; }")
	TEST(test_executor, "variant_assign4",      "export int main() { int a = 1; int b = 2; { a = b = 3; } return a; }")
	TEST(test_executor, "variant_assign5",      "export int main() { int a = 1; int b = 2; int c = 3; a = b = c = 4; return a; }")
	TEST(test_executor, "add_expression1",      "int a = 1 + 1; export int main() { return a; }")
	TEST(test_executor, "add_expression2",      "int a = 1 + 2 + 3; int b = a + 4; export int main() { return b; }")
	TEST(test_executor, "plus_expression1",     "int a = +1; int b = +a; export int main() { return b; }")
	TEST(test_executor, "plus_expression2",     "int a = +foo(); int foo() { return 1; } export int main() { return a; }")
	TEST(test_executor, "sub_expression1",      "int a = 1 - 1; export int main() { return a; }")
	TEST(test_executor, "sub_expression2",      "int a = 3 - 2 - 1; export int main() { return a; }")
	TEST(test_executor, "minus_expression1",    "int a = -1; int b = -a; export int main() { return b; }")
	TEST(test_executor, "minus_expression2",    "int a = -1; int b = -a; int c = foo(); int foo() { return 0; } export int main() { return b; }")
	TEST(test_executor, "mul_expression1",      "int a = 2 * 2; int b = a * 2; export int main() { return b; }")
	TEST(test_executor, "mul_expression2",      "int a = 2 * foo(); int foo() { return 2; } export int main() { return a; }")
	TEST(test_executor, "div_expression1",      "int a =  10 / 2; export int main() { return a; }")
	TEST(test_executor, "div_expression2",      "int a =  18 / 2 / 3; export int main() { return a; }")
	TEST(test_executor, "div_expression3",      "int a =  18 / 2; int b = a / 3; export int main() { return b; }")
	TEST(test_executor, "mod_expression1",      "int a =  18 % 4; export int main() { return a; }")
	TEST(test_executor, "mod_expression2",      "int a =  2 * 9 % 4; export int main() { return a; }")
	TEST(test_executor, "mod_expression3",      "int a =  2 * 9; int b = 4; int c = a % b; export int main() { return c; }")
	TEST(test_executor, "operation1",           "int a = -1; int b = -a; int c = -a + -b; int d = +a - -b; export int main() { return a; }")
	TEST(test_executor, "operation2",           "int a = 1 * 2 + 3 * 4 - 5 * 6; export int main() { return a; }")
	TEST(test_executor, "operation3",           "int a = 1 * 2 + 3 * 4 - 6 / 2; export int main() { return a; }")
	TEST(test_executor, "operation4",           "int a = 1 * 2 + 3 * 4 - 6 / 2 + 19 % 2; export int main() { return a; }")
	TEST(test_executor, "inc_statement1",       "int a = 1; export int main() { a++; return a; }")
	TEST(test_executor, "inc_statement2",       "int a = 1; export int main() { a++; a++; return a; }")
	TEST(test_executor, "dec_statement1",       "int a = 2; export int main() { a--; return a; }")
	TEST(test_executor, "dec_statement2",       "int a = 2; export int main() { a--; a--; return a; }")
	TEST(test_executor, "add_assign_statement1","int a = 0; export int main() { a += 1; return a; }")
	TEST(test_executor, "add_assign_statement2","int a = 0; int b = 1; export int main() { a += b; return a; }")
	TEST(test_executor, "add_assign_statement3","int a = 0; int b = 1; export int main() { a += 3; a += b; return a; }")
	TEST(test_executor, "sub_assign_statement1","int a = 1; export int main() { a -= 1; return a; }")
	TEST(test_executor, "sub_assign_statement2","int a = 2; int b = 1; export int main() { a -= b; return a; }")
	TEST(test_executor, "sub_assign_statement3","int a = 3; int b = 2; export int main() { a += 3; a -= b; return a; }")
	TEST(test_executor, "mul_assign_statement1","int a = 2; export int main() { a *= 2; return a; }")
	TEST(test_executor, "mul_assign_statement2","int a = 3; int b = 3; export int main() { a *= b; return a; }")
	TEST(test_executor, "div_assign_statement1","int a = 4; export int main() { a /= 2; return a; }")
	TEST(test_executor, "div_assign_statement2","int a = 8; int b = 2; export int main() { a /= b; return a; }")
	TEST(test_executor, "mod_assign_statement1","int a = 9; export int main() { a %= 2; return a; }")
	TEST(test_executor, "mod_assign_statement2","int a = 18; int b = 4; export int main() { a %= b; return a; }")
	TEST(test_executor, "not_expression1",      "bool a = !false; export int main() { a; return 0; }")
	TEST(test_executor, "not_expression2",      "bool a = false; bool b = !a; export int main() { a; return 0; }")
	TEST(test_executor, "if_statement1",        "export int main() { if (true) return 1; return 0; }")
	TEST(test_executor, "if_statement2",        "export int main() { if (true) { return 1; } return 0; }")
	TEST(test_executor, "if_statement3",        "export int main() { if (!false) { return 1; } return 0; }")
	TEST(test_executor, "if_statement4",        "export int main() { bool a = true; if (a) { return 1; } return 0; }")
	TEST(test_executor, "if_statement5",        "export int main() { bool a = false; if (!a) { return 1; } return 0; }")
	TEST(test_executor, "if_statement6",        "export int main() { bool a = false; if (a) { return 1; } return 0; }")
	TEST(test_executor, "if_statement6",        "export int main() { bool a = false; if (a) { return 1; } else return 2; return 0; }")
	TEST(test_executor, "if_statement7",        "export int main() { if (true) if (!false) return 1; else return 2; else return 3;  return 0; }")
	TEST(test_executor, "if_statement8",        "export int main() { if (!true) if (true) return 1; else return 2; else return 3;  return 0; }")
	TEST(test_executor, "not_equal_expression", "bool a = 1 != 2; bool b = 1 + 1 != 2; bool c = 1 + 1 != 2 * 2; export int main() { if (!b) return 1; return 0; }")
	TEST(test_executor, "equals_expression",    "bool a = 1 == 2; bool b = 1 + 1 == 2; bool c = 1 + 1 == 3 - 1; bool d = b == false; export int main() { if (d) return 1; return 0; }")
	TEST(test_executor, "less_expression",      "bool a = 1 < 2; bool b = 1 + 1 < 2; bool c = 1 + 1 < 3 - 1; export int main() { if (!c) return 1; return 0; }")
	TEST(test_executor, "great_expression",     "bool a = 2 > 1; bool b = 1 + 1 > 2; bool c = 1 + 1 > 3 - 1; export int main() { if (a) return 1; return 0; }")
	TEST(test_executor, "le_expression",        "bool a = 1 + 1 <= 2; export int main() { if (a) return 1; return 0; }")
	TEST(test_executor, "ge_expression",        "bool a = 2 >= 2; bool b = 1 + 1 >= 2; bool c = 1 + 1 >= 3 - 1; export int main() { if (a) return 1; return 0; }")

	TEST(test_executor, "bitand_expression", "int a = 3 & 1; export int main() { return a; }")

	TEST_WRONG(test_executor, "div_expression_wrong", "int a = 10; int b = 0; export int main() { return a / b; }")
	TEST_WRONG(test_executor, "mod_expression_wrong", "int a = 10; int b = 0; export int main() { return a % b; }")
	TEST_WRONG(test_executor, "div_assign_statement_wrong", "int a = 10; int b = 0; export int main() { a /= b; return 0; }")
	TEST_WRONG(test_executor, "mod_assign_statement_wrong", "int a = 10; int b = 0; export int main() { a %= b; return 0; }")

	TEST(test_generator, "basic",               "export int main() { return 0; }")
	TEST(test_generator, "function_call1",      "int foo() { return 1; } export int main() { return foo(); }")
	TEST(test_generator, "function_call2",      "export int main() { return test1(); } int test1() { return test2(); } int test2() { return 888; }")
	TEST(test_generator, "function_call3",      "export int main() { return bar(); } int bar() { return foo(); } int foo() { return 666; }")
	TEST(test_generator, "global_variant1",     "int a = 0; export int b = 1; export int main() { return b; }")
	TEST(test_generator, "global_variant2",     "int a = foo(); int foo() { return 9527; } export int main() { return a; }")
	TEST(test_generator, "global_variant3",     "export int main() { return a; } int a = 9697;")
	TEST(test_generator, "local_variant1",      "export int main() { int a = 0; int b = a; return a; }")
	TEST(test_generator, "local_variant2",      "export int main() { int a = 0; { int b = a; } int b = 1; return b; }")
	TEST(test_generator, "function_parameter1", "int foo(int a, int b) { return a; }")
	TEST(test_generator, "function_parameter2", "int foo(int a, int b,) { return a; }")
	TEST(test_generator, "function_argument1",  "int foo(int a, int b) { return a; } export int main() { return foo(1, 2); }")
	TEST(test_generator, "function_argument2",  "int foo(int a, int b) { return a; } export int main() { return foo(1, 2,); }")
	TEST(test_generator, "function_argument3",  "int f1(int a) { return a; } int f2(int b) { return b; } int f3(int c) { return c; } export int main() { return f1(f2(f3(4))); }")
	TEST(test_generator, "variant_assign1",     "int a = 0; export int main() { a = 1; return a; }")
	TEST(test_generator, "variant_assign2",     "export int main() { int a = 1; a = 2; return a; }")
	TEST(test_generator, "add_expression1",     "int a = 1 + 1; export int main() { return a; }")
	TEST(test_generator, "add_expression2",     "int a = 1 + 2 + 3; int b = a + 4; export int main() { return b; }")
	TEST(test_generator, "add_expression3",     "int a = foo() + bar(); int foo() { return 1; } int bar() { return 2; } export int main() { return a; }")
	TEST(test_generator, "plus_expression1",    "int a = +1; int b = +a; export int main() { return b; }")
	TEST(test_generator, "plus_expression2",    "int a = +foo(); int foo() { return 1; } export int main() { return a; }")
	TEST(test_generator, "plus_expression3",    "int a = 1 + +foo(); int foo() { return 1; } export int main() { return a; }")
	TEST(test_generator, "sub_expression1",     "int a = 1 - 1; export int main() { return a; }")
	TEST(test_generator, "sub_expression2",     "int a = 3 - 2 - 1; export int main() { return a; }")
	TEST(test_generator, "minus_expression1",   "int a = -1; int b = -a; export int main() { return b; }")
	TEST(test_generator, "minus_expression2",   "int a = -1; int b = -a; int c = foo(); int foo() { return 0; } export int main() { return b; }")
	TEST(test_generator, "mul_expression1",     "int a = 2 * 2; int b = a * 2; export int main() { return b; }")
	TEST(test_generator, "mul_expression2",     "int a = 2 * foo(); int foo() { return 2; } export int main() { return a; }")
	TEST(test_generator, "div_expression1",     "int a =  10 / 2; export int main() { return a; }")
	TEST(test_generator, "div_expression2",     "int a =  18 / 2 / 3; export int main() { return a; }")
	TEST(test_generator, "div_expression3",     "int a =  18 / 2; int b = a / 3; export int main() { return b; }")
	TEST(test_generator, "mod_expression1",     "int a =  18 % 4; export int main() { return a; }")
	TEST(test_generator, "mod_expression2",     "int a =  2 * 9 % 4; export int main() { return a; }")
	TEST(test_generator, "mod_expression3",     "int a =  2 * 9; int b = 4; int c = a % b; export int main() { return c; }")
	TEST(test_generator, "operation1",          "int a = -1; int b = -a; int c = -a + -b; int d = +a - -b; export int main() { return a; }")
	TEST(test_generator, "operation2",          "int a = 1 * 2 + 3 * 4 - 5 * 6; export int main() { return a; }")
	TEST(test_generator, "operation3",          "int a = 1 * 2 + 3 * 4 - 6 / 2; export int main() { return a; }")
	TEST(test_generator, "operation4",          "int a = 1 * 2 + 3 * 4 - 6 / 2 + 19 % 2; export int main() { return a; }")
	TEST(test_generator, "inc_statement1",      "int a = 1; export int main() { a++; return a; }")
	TEST(test_generator, "inc_statement2",      "int a = 1; export int main() { a++; a++; return a; }")
	TEST(test_generator, "dec_statement1",      "int a = 2; export int main() { a--; return a; }")
	TEST(test_generator, "dec_statement2",      "int a = 2; export int main() { a--; a--; return a; }")
	TEST(test_generator, "add_assign_statement1", "int a = 0; export int main() { a += 1; return a; }")
	TEST(test_generator, "add_assign_statement2", "int a = 0; int b = 1; export int main() { a += b; return a; }")
	TEST(test_generator, "add_assign_statement3", "int a = 0; int b = 1; export int main() { a += 3; a += b; return a; }")
	TEST(test_generator, "sub_assign_statement1", "int a = 1; export int main() { a -= 1; return a; }")
	TEST(test_generator, "sub_assign_statement2", "int a = 2; int b = 1; export int main() { a -= b; return a; }")
	TEST(test_generator, "sub_assign_statement3", "int a = 3; int b = 2; export int main() { a += 3; a -= b; return a; }")
	TEST(test_generator, "mul_assign_statement1", "int a = 2; export int main() { a *= 2; return a; }")
	TEST(test_generator, "mul_assign_statement2", "int a = 3; int b = 3; export int main() { a *= b; return a; }")
	TEST(test_generator, "div_assign_statement1", "int a = 4; export int main() { a /= 2; return a; }")
	TEST(test_generator, "div_assign_statement2", "int a = 8; int b = 2; export int main() { a /= b; return a; }")
	TEST(test_generator, "mod_assign_statement1", "int a = 9; export int main() { a %= 2; return a; }")
	TEST(test_generator, "mod_assign_statement2", "int a = 18; int b = 4; export int main() { a %= b; return a; }")
	TEST(test_generator, "not_expression1",       "bool a = !false; export int main() { a; return 0; }")
	TEST(test_generator, "not_expression2",       "bool a = false; bool b = !a; export int main() { a; return 0; }")
	TEST(test_generator, "if_statement1",         "export int main() { if (true) return 1; return 0; }")
	TEST(test_generator, "if_statement2",         "export int main() { if (true) { return 1; } return 0; }")
	TEST(test_generator, "if_statement3",         "export int main() { if (!false) { return 1; } return 0; }")
	TEST(test_generator, "if_statement4",         "export int main() { bool a = true; if (a) { return 1; } return 0; }")
	TEST(test_generator, "if_statement5",         "export int main() { bool a = false; if (!a) { return 1; } return 0; }")
	TEST(test_generator, "if_statement6",         "export int main() { bool a = false; if (a) { return 1; } return 0; }")
	TEST(test_generator, "if_statement7",         "export int main() { if (true) if (!false) return 1; else { return 2; } else return 3;  return 0; }")
	TEST(test_generator, "if_statement8",         "export int main() { if (true) { return 1; } else if (!false) { return 2; } else { return 3; }  return 0; }")
	TEST(test_generator, "not_equal_expression",  "bool a = 1 != 2; bool b = 1 + 1 != 2; bool c = 1 + 1 != 2 * 2; export int main() { if (!b) return 1; return 0; }")
	TEST(test_generator, "less_expression",       "bool a = 1 < 2; bool b = 1 + 1 < 2; bool c = 1 + 1 < 3 - 1; export int main() { if (!c) return 1; return 0; }")
	TEST(test_generator, "great_expression",      "bool a = 2 > 1; bool b = 1 + 1 > 2; bool c = 1 + 1 > 3 - 1; export int main() { if (a) return 1; return 0; }")
	TEST(test_generator, "le_expression",         "bool a = 1 + 1 <= 2; export int main() { if (a) return 1; return 0; }")
	TEST(test_generator, "ge_expression",         "bool a = 2 >= 2; bool b = 1 + 1 >= 2; bool c = 1 + 1 >= 3 - 1; export int main() { if (a) return 1; return 0; }")
	TEST(test_generator, "bitand_expression",     "int a = 3 & 1; export int main() { return a; }")

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
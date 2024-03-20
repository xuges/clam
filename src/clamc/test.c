#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "source.h"
#include "lexer.h"
#include "parser.h"
#include "executor.h"
#include "analyzer.h"

#include "vector.h"
#include "stack.h"
#include "hash_map.h"


void test_vector_add()
{
	printf("testing %s\n", __FUNCTION__);

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

void test_stack()
{
	printf("testing %s\n", __FUNCTION__);

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

void test_string_literal()
{
	printf("testing %s\n", __FUNCTION__);

	String s = String_literal("hello");
	printf("buf=" String_FMT, String_arg(s));
}

void test_string_buffer_basic()
{
	printf("testing %s\n", __FUNCTION__);

	StringBuffer buf;
	StringBuffer_init(&buf);

	StringBuffer_append(&buf, "hello");
	StringBuffer_append(&buf, " world");

	printf("buf=" String_FMT, String_arg(buf));

	StringBuffer_destroy(&buf);
}

void test_string_buffer_reset()
{
	printf("testing %s\n", __FUNCTION__);

	StringBuffer buf;
	StringBuffer_init(&buf);

	StringBuffer_append(&buf, "hello");
	StringBuffer_append(&buf, " world");

	printf("buf=" String_FMT, String_arg(buf));

	StringBuffer_reset(&buf);
	printf("buf.data=%p buf.length=%d buf.cap=%d\n", buf.data, buf.length, buf.cap);

	StringBuffer_append(&buf, "good");

	StringBuffer_destroy(&buf);
}

void test_string_buffer_clone()
{
	printf("testing %s\n", __FUNCTION__);

	StringBuffer buf = String_literal("hello");
	StringBuffer buf1 = StringBuffer_clone(&buf);
	StringBuffer_append(&buf1, " world");
	printf("buf1=" String_FMT, String_arg(buf1));
	StringBuffer_destroy(&buf1);
}

void test_lexer_peek_eof()
{
	printf("testing %s\n", __FUNCTION__);

	Source src;
	Source_init(&src, "");

	Lexer lex;
	Lexer_init(&lex, &src);

	Token* token = Lexer_peek(&lex);
	printf("token.type=%d token.value=%d\n", token->type, token->value);

}

void test_lexer_next_eof()
{
	printf("testing %s\n", __FUNCTION__);

	Source src;
	Source_init(&src, "");

	Lexer lex;
	Lexer_init(&lex, &src);

	Token* token = Lexer_next(&lex);
	printf("token.type=%d token.value=%d\n", token->type, token->value);
}

void test_lexer_print()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return 0; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Token* token = Lexer_next(&lex);
	while (token->value)
	{
		printf("%.*s\n", token->literal.length, token->literal.data);
		token = Lexer_next(&lex);
	}
}

void test_lexer_void_comma()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "void print(int a, int b) {}");

	Lexer lex;
	Lexer_init(&lex, &source);

	Token* token = Lexer_next(&lex);
	while (token->value)
	{
		printf("%.*s\n", token->literal.length, token->literal.data);
		token = Lexer_next(&lex);
	}
}

void test_lexer_assign() {
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = 10;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Token* token = Lexer_next(&lex);
	while (token->value)
	{
		printf("%.*s\n", token->literal.length, token->literal.data);
		token = Lexer_next(&lex);
	}
}

void test_parser_basic()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	printf("module.functions.size=%d\n", module->functions.size);
}

void test_parser_wrong1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { 0 return; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	printf("should not go here\n");
}

void test_parser_wrong2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int export main() { }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	printf("should not go here\n");
}

void test_parser_wrong3()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export main() { }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	printf("should not go here\n");
}

void test_parser_variant1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a;int b = 10;\nexport int c = 666; export int d;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		printf("%d:%d: %s%.*s %.*s\n", decl->location.line, decl->location.colum,
			decl->exported ? "export " : "",
			String_arg(decl->variant.type.name),
			String_arg(decl->variant.name));
	}
}

void test_parser_variant2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int _a;int a2 = 10; export int a_3 = 0;\nexport int __c = 666; export int _d1_;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		printf("%d:%d: %s%.*s %.*s\n", decl->location.line, decl->location.colum,
			decl->exported ? "export " : "",
			String_arg(decl->variant.type.name),
			String_arg(decl->variant.name));
	}
}

void test_parser_variant_wrong()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int 1a = 1;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	printf("should not go here\n");
}

void test_parser_variant_type()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = 0;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	Declaration* decl = Vector_get(&module->declarations, 0);
	printf("%.*s:\n\ttype name: %.*s\n\ttype id: %d\n",
		String_arg(decl->variant.name),
		String_arg(decl->variant.type.name),
		decl->variant.type.id);
}

void test_parser_void_function()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "void print() {}");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	for (int i = 0; i < module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->functions, i);
		printf("%d:%d: %s%.*s %.*s()\n", decl->location.line, decl->location.colum,
			decl->exported ? "export " : "",
			decl->function.resType.name.length, decl->function.resType.name.data,
			decl->function.name.length, decl->function.name.data);
	}
}

void test_parser_functions()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int foo(){}\n int bar(){}\nint print(){}\nexport int main() { return 0; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	for (int i = 0; i < module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->functions, i);
		printf("%d:%d: %s%.*s %.*s()\n", decl->location.line, decl->location.colum,
			decl->exported ? "export " : "",
			decl->function.resType.name.length, decl->function.resType.name.data,
			decl->function.name.length, decl->function.name.data);
	}
}

void test_parser_return_int()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return 666; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Declaration* decl = (Declaration*)module->functions.data;    //module.functions[0]
	Statement* stat = (Statement*)decl->function.block.data;     //functions[0].block[0]
	printf("main return %d\n", stat->returnExpr->intExpr);        //block[0].returnExpr.intExpr
}

void test_parser_functions_return_int()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int foo(){return 1;}\nint bar(){return 2;}\nexport int main() { return 666; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	for (int i = 0; i < module->functions.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->functions, i);
		Statement* stat = (Statement*)decl->function.block.data;

		printf("%d:%d: %s%.*s %.*s() return %d\n", decl->location.line, decl->location.colum,
			decl->exported ? "export " : "",
			decl->function.resType.name.length, decl->function.resType.name.data,
			decl->function.name.length, decl->function.name.data,
			stat->returnExpr->intExpr);
	}
}

void test_parser_multi_block()
{
	printf("testing %s\n", __FUNCTION__);
	Source source;
	Source_init(&source, "export int main()\n{\n\tint a = 1;\n\t{\n\t\tint b = 2;\n\t}\n\treturn a;\n}");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);
	Declaration* mainDecl = Vector_get(&module->functions, 0);
	Vector mainBlock = mainDecl->function.block;
	Statement* blockStat = Vector_get(&mainBlock, 1);
	Statement* bStat = Vector_get(&blockStat->compound, 0);
	Declaration* bDecl = &bStat->declaration;
	printf("%d:%d %.*s %.*s\n", bDecl->location.line, bDecl->location.colum,
		String_arg(bDecl->variant.type.name),
		String_arg(bDecl->variant.name));
}

void test_executor_basic()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return 12345; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_wrong_main1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int main() { return 12345; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_wrong_main2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export void main() { return 0; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_void_function()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "void foo() { }\n export int main() { foo(); return 0; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_function_no_return()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_function_call1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int foo() { return 1; }\nexport int main() { return foo(); }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_function_call2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int foo() { return 666; }\nint bar() { return foo(); }\nexport int main() { return bar(); }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_function_call3()
{
	printf("testing %s\n", __FUNCTION__);

	char* code =
		"export int main() { return bar(); }\n"
		"int bar() { return foo(); }\n"
		"int foo() { return 666; }";

	Source source;
	Source_init(&source, code);

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_global_variant1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = 666; export int main() { return a; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_global_variant2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return a; }\nint a = 1234;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_global_variant_wrong1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = 0;\nexport int main() { return b; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_local_variant1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { int a = 888; return a; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_local_variant2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { int a = 666; { return a; } }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_local_variant3()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { int a = 123; { int b = a; return b; } }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_local_variant4()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { int a = 1; { int a = 2; return a; } }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_local_variant_wrong1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { { int a = 2; } return a; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_executor_global_variant_init1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = foo(); int foo() { return 1234; } export int main() { return a; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Executor exec;
	Executor_init(&exec);
	Executor_run(&exec, module);
}

void test_analyzer_basic()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return 12345; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("test %s over.\n", __FUNCTION__);
}

void test_analyzer_wrong1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int main() { return 0; }");  //syntax normal, semantic wrong

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("should not go here\n");
}

//error: function 'main' must return int
void test_analyzer_wrong2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export void main() { return 0; }");  //syntax normal, semantic wrong

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("should not go here\n");
}

//error: undefined function foo
void test_analyzer_wrong3()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return foo(); }");  //syntax normal, semantic wrong

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("should not go here\n");
}

void test_analyzer_global_variant1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int a;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("test %s over.\n", __FUNCTION__);
}

void test_analyzer_global_variant_wrong1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int a = 0; int a = 1;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("test %s over.\n", __FUNCTION__);
}

void test_analyzer_global_variant_wrong2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "void a;");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("test %s over.\n", __FUNCTION__);
}

void test_analyzer_global_variant_wrong3()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = \"1\";");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("test %s over.\n", __FUNCTION__);
}

void test_analyzer_local_variant1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { int a = 0; { int b = a; } return a; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	printf("test %s over.\n", __FUNCTION__);
}


void test_generator_basic()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "export int main() { return 12345; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	Generator gen;
	Generator_init(&gen, GENERATE_TARGE_C);

	Generator_generate(&gen, module);

	StringBuffer buf;
	StringBuffer_init(&buf);

	Generator_getSource(&gen, &buf);

	printf("generate header:\n" String_FMT "\n\n", String_arg(gen.header));
	printf("generate source:\n" String_FMT "\n\n", String_arg(buf));
}

void test_generator_function_call2()
{
	printf("testing %s\n", __FUNCTION__);

	char* code =
		"export int main() { return test1(); }\n"
		"int test1() { return test2(); }\n"
		"int test2() { return 888; }";

	Source source;
	Source_init(&source, code);

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	Generator gen;
	Generator_init(&gen, GENERATE_TARGE_C);

	Generator_generate(&gen, module);

	StringBuffer buf;
	StringBuffer_init(&buf);

	Generator_getSource(&gen, &buf);

	printf("generate header:\n" String_FMT "\n\n", String_arg(gen.header));
	printf("generate source:\n" String_FMT "\n\n", String_arg(buf));
}

void test_generator_function_call3()
{
	printf("testing %s\n", __FUNCTION__);

	char* code =
		"export int main() { return bar(); }\n"
		"int bar() { return foo(); }\n"
		"int foo() { return 666; }";

	Source source;
	Source_init(&source, code);

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	Generator gen;
	Generator_init(&gen, GENERATE_TARGE_C);

	Generator_generate(&gen, module);

	StringBuffer buf;
	StringBuffer_init(&buf);

	Generator_getSource(&gen, &buf);

	printf("generate header:\n" String_FMT "\n\n", String_arg(gen.header));
	printf("generate source:\n" String_FMT "\n\n", String_arg(buf));
}

void test_generator_global_variant1()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = 0; export int b = 1; export int main() { return b; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	Generator gen;
	Generator_init(&gen, GENERATE_TARGE_C);

	Generator_generate(&gen, module);

	StringBuffer buf;
	StringBuffer_init(&buf);

	Generator_getSource(&gen, &buf);

	printf("generate header:\n" String_FMT "\n\n", String_arg(gen.header));
	printf("generate source:\n" String_FMT "\n\n", String_arg(buf));
}

void test_generator_global_variant2()
{
	printf("testing %s\n", __FUNCTION__);

	Source source;
	Source_init(&source, "int a = foo(); int foo() { return 9527; } export int main() { return a; }");

	Lexer lex;
	Lexer_init(&lex, &source);

	Parser parser;
	Parser_init(&parser);

	Module* module = Parser_translate(&parser, &lex);

	Analyzer anly;
	Analyzer_init(&anly);

	Analyzer_analyse(&anly, module);

	Generator gen;
	Generator_init(&gen, GENERATE_TARGE_C);

	Generator_generate(&gen, module);

	StringBuffer buf;
	StringBuffer_init(&buf);

	Generator_getSource(&gen, &buf);

	printf("generate header:\n" String_FMT "\n\n", String_arg(gen.header));
	printf("generate source:\n" String_FMT "\n\n", String_arg(buf));
}

typedef void(*test_fn)();
test_fn tests[] =
{
	test_vector_add,
	test_stack,
	test_string_literal,
	test_string_buffer_basic,
	test_string_buffer_reset,
	test_string_buffer_clone,
	test_lexer_peek_eof,
	test_lexer_print,
	test_lexer_void_comma,
	test_lexer_assign,
	test_parser_basic,
	test_parser_wrong1,
	test_parser_wrong2,
	test_parser_wrong3,
	test_parser_variant1,
	test_parser_variant2,
	test_parser_variant_wrong,
	test_parser_variant_type,
	test_parser_void_function,
	test_parser_functions,
	test_parser_return_int,
	test_parser_functions_return_int,
	test_parser_multi_block,
	test_executor_basic,
	test_executor_wrong_main1,
	test_executor_wrong_main2,
	test_executor_void_function,
	test_executor_function_no_return,
	test_executor_function_call1,
	test_executor_function_call2,
	test_executor_function_call3,
	test_executor_global_variant1,
	test_executor_global_variant2,
	test_executor_global_variant_wrong1,
	test_executor_local_variant1,
	test_executor_local_variant2,
	test_executor_local_variant3,
	test_executor_local_variant4,
	test_executor_local_variant_wrong1,
	test_executor_global_variant_init1,
	test_analyzer_basic,
	test_analyzer_wrong1,
	test_analyzer_wrong2,
	test_analyzer_wrong3,
	test_analyzer_global_variant1,
	test_analyzer_global_variant_wrong1,
	test_analyzer_global_variant_wrong2,
	test_analyzer_global_variant_wrong3,
	test_analyzer_local_variant1,
	test_generator_basic,
	test_generator_function_call2,
	test_generator_function_call3,
	test_generator_global_variant1,
	test_generator_global_variant2,
};

int main(int argc, char** argv)
{
	//test_generator_global_variant1(); return 0;

	if (argc > 1)
	{
		int i = atoi(argv[1]);
		tests[i]();
		return 0;
	}

	char cmd[512];
	int pos = sprintf(cmd, "%s ", argv[0]);

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
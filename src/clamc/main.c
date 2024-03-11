#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "source.h"
#include "lexer.h"
#include "parser.h"
#include "executor.h"
#include "analyzer.h"

#include "vector.h"
#include "stack.h"

static const char* ver = "v0.0.1";

void usage()
{
	printf(
		"usage: clamc [options] [file]\n"
		"-r\trun code\n"
		"-h\tshow usage\n"
		"-v\tshow version\n"		
	);
}

void version()
{
	printf("clamc version %s\n", ver);
}

int main(int argc, char** argv)
{
	bool run = false;

	//options
	--argc;
	++argv;
	while (argc)
	{
		if (strcmp(*argv, "-h") == 0)
			usage();
		else if (strcmp(*argv, "-v") == 0)
			version();
		else if (strcmp(*argv, "-r") == 0)
			run = true;
		else if ((*argv)[0] == '-')
		{
			printf("unknown options '%s'\n", *argv);
			return -1;
		}
		else
			break;

		--argc;
		++argv;
	}

	if (!argc)
		return 0;

	Source src;
	if (!Source_open(&src, *argv))
	{
		printf("open '%s' failed\n", *argv);
		return -1;
	}

	Lexer lex;
	Lexer_init(&lex, &src);

	Parser p;
	Parser_init(&p);

	Module* mod = Parser_translate(&p, &lex);

	if (run)
	{
		Executor exec;
		Executor_init(&exec);
		Executor_run(&exec, mod);
	}
	else
	{
		Generator gen;
		Generator_init(&gen, GENERATE_TARGE_C);

		Analyzer anly;
		Analyzer_init(&anly);
		Analyzer_generate(&anly, mod, &gen);

		printf(String_FMT, String_arg(gen.declarations));
		printf(String_FMT, String_arg(gen.definitions));
	}

	return 0;
}
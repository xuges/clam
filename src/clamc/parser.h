#ifndef CLAM_PARSER_H
#define CLAM_PARSER_H

#include "ast.h"
#include "lexer.h"
#include "module.h"

struct Parser
{
	Module module;
	Lexer* lex;
};
typedef struct Parser Parser;

bool Parser_init(Parser* p);
void Parser_destroy(Parser* p);
Module* Parser_translate(Parser* p, Lexer* lex);

#endif
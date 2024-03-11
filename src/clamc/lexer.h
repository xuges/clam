#ifndef CLAM_LEXER_H
#define CLAM_LEXER_H

#include "source.h"
#include "token.h"

struct Lexer {
	SourceLocation location;
	Token token;    //current token
	Source* source;
};
typedef struct Lexer Lexer;

void Lexer_init(Lexer* lex, Source* source);
void Lexer_destroy(Lexer* lex);
Token* Lexer_peek(Lexer* lex);
Token* Lexer_next(Lexer* lex);

#endif
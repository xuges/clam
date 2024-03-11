#include <stdlib.h>

#include "token.h"

void Token_init(Token* token, const char* filename)
{
	String_init(&token->literal);
	token->location.filename = filename;
	token->location.line = 0;
	token->location.colum = 0;
	token->type = TOKEN_TYPE_EOF;
	token->value = TOKEN_VALUE_EOF;
	token->base = TOKEN_NUM_INIT;
}

void Token_destroy(Token* token)
{
	
}

void Token_reset(Token* token, SourceLocation loc, TokenType type, TokenValue value)
{
	String_init(&token->literal);

	token->location = loc;

	token->type = type;
	token->value = value;
	token->base = TOKEN_NUM_DEC;
}

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "lexer.h"
#include "message.h"

struct Keyword
{
	String literal;
	TokenType   type;
	TokenValue  value;
};
typedef struct Keyword Keyword;

static Keyword keywords[] =
{
	{ String_literal("void"  ), TOKEN_TYPE_KEYWORD_TYPE, TOKEN_VALUE_VOID   },
	{ String_literal("int"   ), TOKEN_TYPE_KEYWORD_TYPE, TOKEN_VALUE_INT    },
	{ String_literal("bool"  ), TOKEN_TYPE_KEYWORD_TYPE, TOKEN_VALUE_BOOL   },
	{ String_literal("true"  ), TOKEN_TYPE_BOOL,         TOKEN_VALUE_TRUE   },
	{ String_literal("false" ), TOKEN_TYPE_BOOL,         TOKEN_VALUE_FALSE  },
	{ String_literal("if"    ), TOKEN_TYPE_KEYWORD,      TOKEN_VALUE_IF     },
	{ String_literal("else"  ), TOKEN_TYPE_KEYWORD,      TOKEN_VALUE_ELSE   },
	{ String_literal("export"), TOKEN_TYPE_KEYWORD,      TOKEN_VALUE_EXPORT },
	{ String_literal("return"), TOKEN_TYPE_KEYWORD,      TOKEN_VALUE_RETURN },
};

static bool _Lexer_isEof(Lexer* lex);
static const char* _Lexer_peek(Lexer* lex);
static const char* _Lexer_next(Lexer* lex);

static void _Lexer_skipSpace(Lexer* lex);
static void _Lexer_parseIdent(Lexer* lex);
static void _Lexer_parseNumber(Lexer* lex);
static void _Lexer_parseOperator(Lexer* lex, TokenValue value);
static void _Lexer_parseDelimiter(Lexer* lex, TokenValue value);
static void _Lexer_parseLiteralString(Lexer* lex);
static void _Lexer_parseKeyword(Lexer* lex);

void Lexer_init(Lexer* lex, Source* source)
{
	lex->location.filename = source->name;
	lex->location.line = 1;
	lex->location.colum = 1;
	Token_init(&lex->token, lex->location.filename);
	lex->source = source;

	//Lexer_next(lex);
}

void Lexer_destroy(Lexer* lex)
{
	
}

Token* Lexer_peek(Lexer* lex)
{
	return &lex->token;
}

Token* Lexer_next(Lexer* lex)
{
	do  //parse
	{
		_Lexer_skipSpace(lex);

		if (_Lexer_isEof(lex))
		{
			Token_reset(&lex->token, lex->location, TOKEN_TYPE_EOF, TOKEN_VALUE_EOF);
			break;
		}

		char c = *_Lexer_peek(lex);

		if (isalpha(c) || c == '_')
		{
			_Lexer_parseIdent(lex);
		}
		else if (isdigit(c))
		{
			_Lexer_parseNumber(lex);
		}
		else if (c == '=')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_ASSIGN);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_EQ;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '+')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_ADD);
			c = *_Lexer_peek(lex);
			if (c == '+')
			{
				lex->token.value = TOKEN_VALUE_INC;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
			else if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_ADD_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '-')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_SUB);
			c = *_Lexer_peek(lex);
			if (c == '-')
			{
				lex->token.value = TOKEN_VALUE_DEC;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
			else if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_SUB_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '*')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_STAR);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_MUL_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '/')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_DIV);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_DIV_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '%')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_MOD);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_MOD_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '!')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_NOT);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_NE;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '<')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_LT);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_LE;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
			else if (c == '<')
			{
				lex->token.value = TOKEN_VALUE_LSHIFT;
				lex->token.literal.length++;
				_Lexer_next(lex);

				c = *_Lexer_peek(lex);
				if (c == '=')
				{
					lex->token.value = TOKEN_VALUE_LSHIFT_ASSIGN;
					lex->token.literal.length++;
					_Lexer_next(lex);
				}
			}
		}
		else if (c == '>')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_GT);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_GE;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
			else if (c == '>')
			{
				lex->token.value = TOKEN_VALUE_RSHIFT;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '&')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_BITAND);
			c = *_Lexer_peek(lex);
			if (c == '&')
			{
				lex->token.value = TOKEN_VALUE_AND;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
			else if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_BITAND_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '|')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_BITOR);
			c = *_Lexer_peek(lex);
			if (c == '|')
			{
				lex->token.value = TOKEN_VALUE_OR;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
			else if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_BITOR_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '^')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_XOR);
			c = *_Lexer_peek(lex);
			if (c == '=')
			{
				lex->token.value = TOKEN_VALUE_XOR_ASSIGN;
				lex->token.literal.length++;
				_Lexer_next(lex);
			}
		}
		else if (c == '~')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_NEG);
		}
		else if (c == '?')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_QUES);
		}
		else if (c == ':')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_COLON);
		}
		else if (c == '(')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_LP);
		}
		else if (c == ')')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_RP);
		}
		else if (c == '{')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_LC);
		}
		else if (c == '}')
		{
			_Lexer_parseOperator(lex, TOKEN_VALUE_RC);
		}
		else if (c == ',')
		{
			_Lexer_parseDelimiter(lex, TOKEN_VALUE_COMMA);
		}
		else if (c == ';')
		{
			_Lexer_parseDelimiter(lex, TOKEN_VALUE_SEM);
		}
		else if (c == '"')
		{
			_Lexer_parseLiteralString(lex);
		}
		else
		{
			error(&lex->location, "unknown character '%c'", c);
		}

	} while (0);

	return &lex->token;
}

bool _Lexer_isEof(Lexer* lex)
{
	return Source_isEof(lex->source);
}

const char* _Lexer_peek(Lexer* lex)
{
	return Source_peek(lex->source);
}

const char* _Lexer_next(Lexer* lex)
{
	const char* s = Source_next(lex->source);
	lex->location.colum++;

	if (*s == '\n')
	{
		lex->location.line++;
		lex->location.colum = 1;
	}

	return s;
}

void _Lexer_skipSpace(Lexer* lex)
{
	while (!_Lexer_isEof(lex) && isspace(*_Lexer_peek(lex)))
		_Lexer_next(lex);
}

void _Lexer_parseIdent(Lexer* lex)
{
	Token_reset(&lex->token, lex->location, TOKEN_TYPE_IDENT, TOKEN_VALUE_IDENT);
	lex->token.literal.data = _Lexer_peek(lex);

	const char* s;
	do
	{
		lex->token.literal.length++;
		_Lexer_next(lex);
		s = _Lexer_peek(lex);
	} while (!_Lexer_isEof(lex) && (isalnum(*s) || *s == '_'));

	_Lexer_parseKeyword(lex);
}

void _Lexer_parseNumber(Lexer* lex)
{
	Token_reset(&lex->token, lex->location, TOKEN_TYPE_INT, TOKEN_VALUE_LITERAL_INT);
	lex->token.literal.data = _Lexer_peek(lex);

	//TODO: support float bin hex oct and postfix

	const char* s;
	do
	{
		lex->token.literal.length++;
		_Lexer_next(lex);
		s = _Lexer_peek(lex);
	} while (!_Lexer_isEof(lex) && isdigit(*s));

}

void _Lexer_parseOperator(Lexer* lex, TokenValue value)
{
	Token_reset(&lex->token, lex->location, TOKEN_TYPE_OPERATOR, value);
	lex->token.literal.data = _Lexer_next(lex);
	lex->token.literal.length++;
}

void _Lexer_parseKeyword(Lexer* lex)
{
	//TODO: optimize to hash
	for (int i = 0; i < sizeof(keywords) / sizeof(Keyword); i++)
	{
		Keyword* kw = &keywords[i];
		if (String_equalsString(lex->token.literal, kw->literal))
		{
			lex->token.type  = kw->type;
			lex->token.value = kw->value;
			break;
		}
	}
}

void _Lexer_parseDelimiter(Lexer* lex, TokenValue value)
{
	Token_reset(&lex->token, lex->location, TOKEN_TYPE_DELIMITER, value);
	lex->token.literal.data = _Lexer_next(lex);
	lex->token.literal.length++;
}

void _Lexer_parseLiteralString(Lexer* lex)
{
	Token_reset(&lex->token, lex->location, TOKEN_TYPE_LITERAL, TOKEN_VALUE_LITERAL_STRING);
	_Lexer_next(lex);

	lex->token.literal.data = _Lexer_peek(lex);

	char c;
	while (!_Lexer_isEof(lex))
	{
		c = *_Lexer_next(lex);
		if (c == '"' || c == '\n')
			break; //TODO: parse escape char
		else
			lex->token.literal.length++;
	}

	if (c != '"')
		error(&lex->location, "missing termination \" charactor");
}


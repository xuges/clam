#ifndef CLAM_TOKEN_H
#define CLAM_TOKEN_H

#include "source_location.h"
#include "strings.h"

enum TokenType
{
	TOKEN_TYPE_EOF,

	TOKEN_TYPE_INT,
	TOKEN_TYPE_FLOAT,
	TOKEN_TYPE_BOOL,
	TOKEN_TYPE_LITERAL,

	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_KEYWORD_TYPE,
	TOKEN_TYPE_OPERATOR,
	TOKEN_TYPE_DELIMITER,
	TOKEN_TYPE_IDENT,
};
typedef enum TokenType TokenType;

enum TokenValue
{
	TOKEN_VALUE_EOF,

	TOKEN_VALUE_LP,              // (
	TOKEN_VALUE_RP,              // )
	TOKEN_VALUE_LC,              // {
	TOKEN_VALUE_RC,              // }

	TOKEN_VALUE_ASSIGN,          // =
	TOKEN_VALUE_ADD,             // +
	TOKEN_VALUE_SUB,             // -
	TOKEN_VALUE_STAR,            // *
	TOKEN_VALUE_DIV,             // /
	TOKEN_VALUE_MOD,             // %

	TOKEN_VALUE_ADD_ASSIGN,      // +=
	TOKEN_VALUE_SUB_ASSIGN,      // -=
	TOKEN_VALUE_MUL_ASSIGN,      // *=
	TOKEN_VALUE_DIV_ASSIGN,      // /=
	TOKEN_VALUE_MOD_ASSIGN,      // %=

	TOKEN_VALUE_INC,             // ++
	TOKEN_VALUE_DEC,             // --

	TOKEN_VALUE_NOT,             // !
	TOKEN_VALUE_NE,              // !=
	TOKEN_VALUE_EQ,              // ==
	TOKEN_VALUE_LT,              // <
	TOKEN_VALUE_LE,              // <=
	TOKEN_VALUE_GT,              // >
	TOKEN_VALUE_GE,              // >=
	TOKEN_VALUE_AND,             // &&
	TOKEN_VALUE_OR,              // ||

	TOKEN_VALUE_BITAND,          // &
	TOKEN_VALUE_BITOR,           // |
	TOKEN_VALUE_XOR,             // ^
	TOKEN_VALUE_NEG,             // ~
	TOKEN_VALUE_LSHIFT,          // <<
	TOKEN_VALUE_RSHIFT,          // >>

	TOKEN_VALUE_QUES,            // ?
	TOKEN_VALUE_COLON,           // :

	TOKEN_VALUE_COMMA,           // ,
	TOKEN_VALUE_DOT,             // .
	TOKEN_VALUE_SEM,             // ;

	TOKEN_VALUE_LITERAL_INT,     // 123
	TOKEN_VALUE_LITERAL_STRING,  // "abc"

	TOKEN_VALUE_VOID,            // void
	TOKEN_VALUE_INT,             // int
	TOKEN_VALUE_BOOL,            // bool
	TOKEN_VALUE_TRUE,            // true
	TOKEN_VALUE_FALSE,           // false

	TOKEN_VALUE_IF,              // if
	TOKEN_VALUE_ELSE,            // else

	TOKEN_VALUE_EXPORT,          // export
	TOKEN_VALUE_RETURN,          // return

	TOKEN_VALUE_IDENT,
};
typedef enum TokenValue TokenValue;

enum TokenNumBase
{
	TOKEN_NUM_INIT = 0,
	TOKEN_NUM_OCT = 8,
	TOKEN_NUM_DEC = 10,
	TOKEN_NUM_HEX = 16,
};
typedef enum TokenNumBase TokenNumBase;

struct Token
{
	SourceLocation location;
	String literal;
	TokenType    type;
	TokenValue   value;
	TokenNumBase base;
};
typedef struct Token Token;

void Token_init(Token* token, const char* filename);
void Token_destroy(Token* token);
void Token_reset(Token* token, SourceLocation loc, TokenType type, TokenValue value);

#endif
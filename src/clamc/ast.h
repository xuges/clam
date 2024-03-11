#ifndef CLAM_AST_H
#define CLAM_AST_H

#include <stdbool.h>

#include "type.h"
#include "vector.h"
#include "strings.h"
#include "source_location.h"

//declaration

enum DeclType
{
	DECL_TYPE_INIT,
	DECL_TYPE_FUNCTION,
};
typedef enum DeclType DeclType;

struct Parameter
{
	String name;
	Type type;
};
typedef struct Parameter Parameter;

struct FuncDecl
{
	Type resType;
	String name;
	Vector parameters;  //Vector<Parameter>
	Vector block;       //Vector<Statement>
};
typedef struct FuncDecl FuncDecl;

void FuncDecl_init(FuncDecl* func);

struct Declaration
{
	SourceLocation location;
	DeclType type;
	bool exported;
	union
	{
		FuncDecl function;
	};
};
typedef struct Declaration Declaration;

void Declaration_init(Declaration* decl);


//expression

enum ExprType
{
	EXPR_TYPE_INT,
	EXPR_TYPE_CALL,
	EXPR_TYPE_IDENT,
};
typedef enum ExprType ExprType;

struct CallExpression
{
	struct Expression* func;

};
typedef struct CallExpression CallExpression;

struct Expression
{
	SourceLocation location;
	ExprType type;
	union
	{
		int intExpr;
		CallExpression callExpr;
		String identExpr;
	};
};
typedef struct Expression Expression;

Expression* Expression_createLiteral(ExprType type, Token* token);
Expression* Expression_createCall(SourceLocation* loc, Expression* func);   //TODO: add args list
Expression* Expression_createIdent(SourceLocation* loc, Token* token);

void Expression_destroy(Expression* expr);

//statement

enum StatementType
{
	STATEMENT_TYPE_RETURN,
	STATEMENT_TYPE_RETURN_EXPR,
	STATEMENT_TYPE_EXPRESSION,
};
typedef enum StatementType StatementType;

struct Statement
{
	SourceLocation location;
	StatementType type;
	union
	{
		Expression* returnExpr;
		Expression* expr;
	};
};
typedef struct Statement Statement;

void Statement_destroy(Statement* stat);



#endif
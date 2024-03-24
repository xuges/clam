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
	DECL_TYPE_VARIANT,
};
typedef enum DeclType DeclType;

struct Parameter
{
	String name;
	Type type;
};
typedef struct Parameter Parameter;

void Parameter_init(Parameter* param);

struct FuncDecl
{
	Type resType;
	String name;
	Vector parameters;  //Vector<Parameter>
	Vector block;  //Vector<Statement>
};
typedef struct FuncDecl FuncDecl;

void FuncDecl_init(FuncDecl* func);
void FuncDecl_destroy(FuncDecl* func);


struct VarDecl
{
	Type type;
	String name;
	struct Expression* initExpr;
};
typedef struct VarDecl VarDecl;

void VarDecl_destroy(VarDecl* vd);

struct Declaration
{
	SourceLocation location;
	DeclType type;
	bool exported;
	union
	{
		struct
		{
			Type baseType;
			String name;
		};
		FuncDecl function;
		VarDecl  variant;
	};
};
typedef struct Declaration Declaration;

void Declaration_init(Declaration* decl);
void Declaration_destroy(Declaration* decl);


//expression

enum ExprType
{
	EXPR_TYPE_INT,
	EXPR_TYPE_CALL,
	EXPR_TYPE_IDENT,
	EXPR_TYPE_ASSIGN,
	EXPR_TYPE_PLUS,   //unary
	EXPR_TYPE_MINUS,
	EXPR_TYPE_ADD,    //binary
	EXPR_TYPE_SUB,
};
typedef enum ExprType ExprType;

struct BinaryExpression
{
	struct Expression* leftExpr;
	struct Expression* rightExpr;
};
typedef struct BinaryExpression BinaryExpression;

typedef struct BinaryExpression AssignExpression;

struct CallExpression
{
	struct Expression* func;
	Vector args;  //Vector<Expression>

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
		AssignExpression assignExpr;
		struct Expression* unaryExpr;
		BinaryExpression binaryExpr;
	};
};
typedef struct Expression Expression;

Expression* Expression_createLiteral(ExprType type, Token* token);
Expression* Expression_createCall(SourceLocation* loc, Expression* func);
Expression* Expression_createIdent(SourceLocation* loc, Token* token);
Expression* Expression_createAssign(SourceLocation* loc, ExprType type, Expression* left, Expression* right);
Expression* Expression_createUnary(SourceLocation* loc, ExprType type, Expression* right);
Expression* Expression_createBinary(SourceLocation* loc, ExprType type, Expression* left, Expression* right);

void Expression_destroy(Expression* expr);

//statement

enum StatementType
{
	STATEMENT_TYPE_EMPTY,
	STATEMENT_TYPE_RETURN,
	STATEMENT_TYPE_EXPRESSION,
	STATEMENT_TYPE_COMPOUND,
	STATEMENT_TYPE_DECLARATION,
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
		Vector compound;
		Declaration declaration;
	};
};
typedef struct Statement Statement;

void Statement_init(Statement* stat);
void Statement_destroy(Statement* stat);



#endif
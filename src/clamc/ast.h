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
	Vector block;       //Vector<Statement>
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
	EXPR_TYPE_BOOL,
	EXPR_TYPE_CALL,
	EXPR_TYPE_IDENT,
	EXPR_TYPE_PLUS,   //unary
	EXPR_TYPE_MINUS,
	EXPR_TYPE_NOT,
	EXPR_TYPE_ADD,    //binary
	EXPR_TYPE_SUB,
	EXPR_TYPE_MUL,
	EXPR_TYPE_DIV,
	EXPR_TYPE_MOD,
	EXPR_TYPE_NE,
	EXPR_TYPE_EQ,
	EXPR_TYPE_LT,
	EXPR_TYPE_LE,
	EXPR_TYPE_GT,
	EXPR_TYPE_GE,
	EXPR_TYPE_AND,
	EXPR_TYPE_OR,
	EXPR_TYPE_BITAND,
	EXPR_TYPE_BITOR,
	EXPR_TYPE_XOR,
	EXPR_TYPE_NEG,
	EXPR_TYPE_LSHIFT,
	EXPR_TYPE_RSHIFT,
	EXPR_TYPE_COND,  //ternary
};
typedef enum ExprType ExprType;

struct BinaryExpression
{
	struct Expression* leftExpr;
	struct Expression* rightExpr;
};
typedef struct BinaryExpression BinaryExpression;

struct ConditionExpression
{
	struct Expression* expr1;
	struct Expression* expr2;
	struct Expression* expr3;
};
typedef struct ConditionExpression ConditionExpression;

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
		bool boolExpr;
		CallExpression callExpr;
		String identExpr;
		struct Expression* unaryExpr;
		BinaryExpression binaryExpr;
		ConditionExpression condExpr;
	};
};
typedef struct Expression Expression;

Expression* Expression_create(ExprType type, SourceLocation* loc);
Expression* Expression_createLiteral(ExprType type, Token* token);
Expression* Expression_createCall(SourceLocation* loc, Expression* func);
Expression* Expression_createIdent(SourceLocation* loc, Token* token);
Expression* Expression_createUnary(SourceLocation* loc, ExprType type, Expression* right);
Expression* Expression_createBinary(SourceLocation* loc, ExprType type, Expression* left, Expression* right);

void Expression_destroy(Expression* expr);

bool Expression_contains(Expression* expr, ExprType type);

//statement

enum StatementType
{
	STATEMENT_TYPE_EMPTY,
	STATEMENT_TYPE_DECLARATION,
	STATEMENT_TYPE_ASSIGN,
	STATEMENT_TYPE_ADD_ASSIGN,
	STATEMENT_TYPE_SUB_ASSIGN,
	STATEMENT_TYPE_MUL_ASSIGN,
	STATEMENT_TYPE_DIV_ASSIGN,
	STATEMENT_TYPE_MOD_ASSIGN,
	STATEMENT_TYPE_BITAND_ASSIGN,
	STATEMENT_TYPE_BITOR_ASSIGN,
	STATEMENT_TYPE_XOR_ASSIGN,
	STATEMENT_TYPE_INC,
	STATEMENT_TYPE_DEC,
	STATEMENT_TYPE_IF,
	STATEMENT_TYPE_RETURN,
	STATEMENT_TYPE_EXPRESSION,
	STATEMENT_TYPE_COMPOUND,
};
typedef enum StatementType StatementType;

struct AssignStatement
{
	Expression* leftExpr;
	Expression* rightExpr;
};
typedef struct AssignStatement AssignStatement;

struct IfStatement
{
	Expression* condition;
	struct Statement* statement;
	struct Statement* elseStat;
};
typedef struct IfStatement IfStatement;

struct Statement
{
	SourceLocation location;
	StatementType type;
	union
	{
		Expression* returnExpr;
		Expression* expr;
		Expression* incExpr;
		Expression* decExpr;
		Vector compound;
		Declaration declaration;
		AssignStatement assign;
		IfStatement ifStat;
	};
};
typedef struct Statement Statement;

void Statement_init(Statement* stat);
void Statement_destroy(Statement* stat);

Statement* Statement_alloc();


#endif
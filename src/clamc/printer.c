#include <stdio.h>
#include <stdlib.h>

#include "printer.h"

static const char* tokenTypeToString[] =
{
	"TOKEN_TYPE_EOF",
	"TOKEN_TYPE_INT",
	"TOKEN_TYPE_FLOAT",
	"TOKEN_TYPE_BOOL",
	"TOKEN_TYPE_LITERAL",
	"TOKEN_TYPE_KEYWORD",
	"TOKEN_TYPE_KEYWORD_TYPE",
	"TOKEN_TYPE_OPERATOR",
	"TOKEN_TYPE_DELIMITER",
	"TOKEN_TYPE_IDENT",
};

static const char* tokenValueToString[] =
{
	"TOKEN_VALUE_EOF",
	"TOKEN_VALUE_LP",
	"TOKEN_VALUE_RP",
	"TOKEN_VALUE_LC",
	"TOKEN_VALUE_RC",
	"TOKEN_VALUE_ASSIGN",
	"TOKEN_VALUE_ADD",
	"TOKEN_VALUE_SUB",
	"TOKEN_VALUE_STAR",
	"TOKEN_VALUE_DIV",
	"TOKEN_VALUE_COMMA",
	"TOKEN_VALUE_DOT",
	"TOKEN_VALUE_SEM",
	"TOKEN_VALUE_LITERAL_INT",
	"TOKEN_VALUE_LITERAL_STRING",
	"TOKEN_VALUE_VOID",
	"TOKEN_VALUE_INT",
	"TOKEN_VALUE_EXPORT",
	"TOKEN_VALUE_RETURN",
	"TOKEN_VALUE_IDENT",
};

static const char* declTypeToString[] =
{
	"DECL_TYPE_INIT",
	"DECL_TYPE_FUNCTION",
	"DECL_TYPE_VARIANT"
};

static const char* exprTypeToString[] =
{
	"EXPR_TYPE_INT",
	"EXPR_TYPE_CALL",
	"EXPR_TYPE_IDENT",
	"EXPR_TYPE_ASSIGN",
	"EXPR_TYPE_PLUS",
	"EXPR_TYPE_MINUS",
	"EXPR_TYPE_ADD",
	"EXPR_TYPE_SUB",
	"EXPR_TYPE_MUL"
};

static const char* statTypeToString[] =
{
	"STATEMENT_TYPE_EMPTY",
	"STATEMENT_TYPE_RETURN",
	"STATEMENT_TYPE_EXPRESSION",
	"STATEMENT_TYPE_COMPOUND",
	"STATEMENT_TYPE_DECLARATION"
};

static const char* typeIdToString[] =
{
	"TYPE_INIT",
	"TYPE_VOID",
	"TYPE_INT",
};

static const char* boolToString[] =
{
	"false",
	"true"
};

#define SC_FMT "%s:%d:%d"
#define SC_arg(sc) (sc).filename, (sc).line, (sc).colum

#define T_FMT "%.*s(%s)"
#define T_arg(t) String_arg((t).name), typeIdToString[(t).id]



static void _Printer_declaration(Printer* p, Declaration* decl);
static void _Printer_function(Printer* p, Declaration* decl);
static void _Printer_statement(Printer* p, Statement* stat);
static void _Printer_expression(Printer* p, Expression* expr);
static void _Printer_binaryExpression(Printer* p, Expression* expr);
static void _Printer_callExpression(Printer* p, Expression* expr);
static void _Printer_indent(Printer* p);

void Printer_init(Printer* p)
{
	p->module = NULL;
	p->level = 0;
}

void Printer_printLex(Printer* p, Lexer* lex)
{
	while (Lexer_next(lex)->value != TOKEN_VALUE_EOF)
	{
		Token* token = Lexer_peek(lex);
		printf(String_FMT "\t(" SC_FMT "\t%s\t%s)\n",
			String_arg(token->literal),
			SC_arg(token->location), tokenTypeToString[token->type], tokenValueToString[token->value]);
	}
}

void Printer_printAst(Printer* p, Module* module)
{
	p->module = module;
	for (int i = 0; i < p->module->declarations.size; ++i)
		_Printer_declaration(p, Vector_get(&p->module->declarations, i));
}

void _Printer_declaration(Printer* p, Declaration* decl)
{
	_Printer_indent(p); printf("(declaration)\n");
	_Printer_indent(p); printf("location=" SC_FMT "\n", SC_arg(decl->location));
	_Printer_indent(p); printf("type=%s\n", declTypeToString[decl->type]);
	_Printer_indent(p); printf("exported=%s\n", boolToString[decl->exported]);

	switch (decl->type)
	{
	case DECL_TYPE_FUNCTION:
		_Printer_indent(p); printf("function=\n");

		p->level++;
		_Printer_function(p, decl);
		p->level--;
		break;

	case DECL_TYPE_VARIANT:
		_Printer_indent(p); printf("variant=\n");
		{
			p->level++;

			_Printer_indent(p); printf("type=" T_FMT "\n", T_arg(decl->variant.type));
			_Printer_indent(p); printf("name=" String_FMT "\n", String_arg(decl->variant.name));
			_Printer_indent(p); printf("initExpr=%s\n", decl->variant.initExpr ? "" : "NULL");

			if (decl->variant.initExpr)
			{
				p->level++;
				_Printer_expression(p, decl->variant.initExpr);
				p->level--;
			}

			p->level--;
		}

		break;
	}
}

void _Printer_function(Printer* p, Declaration* decl)
{
	FuncDecl* func = &decl->function;
	_Printer_indent(p); printf("resType=" T_FMT "\n", T_arg(func->resType));
	_Printer_indent(p); printf("name=" String_FMT "\n", String_arg(func->name));

	_Printer_indent(p); printf("parameters=%s\n", func->parameters.size ? "" : "empty");

	p->level++;
	for (int i = 0; i < func->parameters.size; ++i)
	{
		Parameter* param = Vector_get(&func->parameters, i);
		_Printer_indent(p); printf("type=" T_FMT "\n", T_arg(param->type));
		_Printer_indent(p); printf("name=" String_FMT "\n", String_arg(param->name));
	}
	p->level--;

	_Printer_indent(p); printf("block=%s\n", func->block.size ? "" : "empty");

	p->level++;
	for (int i = 0; i < func->block.size; ++i)
	{
		Statement* stat = Vector_get(&func->block, i);
		_Printer_statement(p, stat);
	}
	p->level--;
}

void _Printer_statement(Printer* p, Statement* stat)
{
	_Printer_indent(p); printf("(statement)\n");
	_Printer_indent(p); printf("location=" SC_FMT "\n", SC_arg(stat->location));
	_Printer_indent(p); printf("type=%s\n", statTypeToString[stat->type]);

	switch (stat->type)
	{
	case STATEMENT_TYPE_RETURN:
		_Printer_indent(p); printf("returnExpr=%s\n", stat->returnExpr ? "" : "NULL");

		if (stat->returnExpr)
		{
			p->level++;
			_Printer_expression(p, stat->returnExpr);
			p->level--;
		}

		break;

	case STATEMENT_TYPE_EXPRESSION:
		_Printer_indent(p); printf("expr=\n");

		p->level++;
		_Printer_expression(p, stat->expr);
		p->level--;
		break;

	case STATEMENT_TYPE_COMPOUND:
		_Printer_indent(p); printf("compound=%s\n", stat->compound.size ? "" : "empty");
		p->level++;
		for (int i = 0; i < stat->compound.size; ++i)
		{
			Statement* subStat = Vector_get(&stat->compound, i);
			_Printer_statement(p, subStat);
		}
		p->level--;
		break;

	case STATEMENT_TYPE_DECLARATION:
		_Printer_indent(p); printf("declaration=\n");
		p->level++;
		_Printer_declaration(p, &stat->declaration);
		p->level--;
		break;
	}
}

void _Printer_expression(Printer* p, Expression* expr)
{
	_Printer_indent(p); printf("(expression)\n");
	_Printer_indent(p); printf("location=" SC_FMT "\n", SC_arg(expr->location));
	_Printer_indent(p); printf("type=%s\n", exprTypeToString[expr->type]);

	switch (expr->type)
	{
	case EXPR_TYPE_INT:
		_Printer_indent(p); printf("intExpr=%d\n", expr->intExpr);
		break;

	case EXPR_TYPE_CALL:
		_Printer_indent(p); printf("callExpr=\n");
		p->level++;
		_Printer_callExpression(p, expr);
		p->level--;
		break;

	case EXPR_TYPE_IDENT:
		_Printer_indent(p); printf("identExpr=" String_FMT "\n", String_arg(expr->identExpr));
		break;

	case EXPR_TYPE_ASSIGN:
		_Printer_indent(p); printf("assignExpr=\n");
		p->level++;
		_Printer_binaryExpression(p, expr);
		p->level--;
		break;

	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
		_Printer_indent(p); printf("unaryExpr=\n");
		p->level++;
		_Printer_expression(p, expr->unaryExpr);
		p->level--;
		break;

	case EXPR_TYPE_ADD:
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
		_Printer_indent(p); printf("binaryExpr=\n");
		p->level++;
		_Printer_binaryExpression(p, expr);
		p->level--;
		break;

	}

}

void _Printer_binaryExpression(Printer* p, Expression* expr)
{
	_Printer_indent(p); printf("leftExpr=\n");
	{
		p->level++;
		_Printer_expression(p, expr->binaryExpr.leftExpr);
		p->level--;
	}
	_Printer_indent(p); printf("rightExpr=\n");
	{
		p->level++;
		_Printer_expression(p, expr->binaryExpr.rightExpr);
		p->level--;
	}
}

void _Printer_callExpression(Printer* p, Expression* expr)
{
	_Printer_indent(p); printf("func=\n");
	p->level++;
	_Printer_expression(p, expr->callExpr.func);
	p->level--;

	_Printer_indent(p); printf("args=%s\n", expr->callExpr.args.size ? "" : "empty");
	
	p->level++;
	for (int i = 0; i < expr->callExpr.args.size; ++i)
	{
		Expression* arg = Vector_get(&expr->callExpr.args, i);
		_Printer_expression(p, arg);
	}
	p->level--;

}

void _Printer_indent(Printer* p)
{
	for (int i = 0; i < p->level; ++i)
		printf("  ");
}

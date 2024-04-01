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
	"TOKEN_VALUE_MOD",
	"TOKEN_VALUE_ADD_ASSIGN",
	"TOKEN_VALUE_SUB_ASSIGN",
	"TOKEN_VALUE_MUL_ASSIGN",
	"TOKEN_VALUE_DIV_ASSIGN",
	"TOKEN_VALUE_MOD_ASSIGN",
	"TOKEN_VALUE_INC",
	"TOKEN_VALUE_DEC",
	"TOKEN_VALUE_NOT",
	"TOKEN_VALUE_NE",
	"TOKEN_VALUE_EQ",
	"TOKEN_VALUE_LT",
	"TOKEN_VALUE_LE",
	"TOKEN_VALUE_GT",
	"TOKEN_VALUE_GE",
	"TOKEN_VALUE_AND",
	"TOKEN_VALUE_OR",
	"TOKEN_VALUE_BITAND",
	"TOKEN_VALUE_BITOR",
	"TOKEN_VALUE_XOR",
	"TOKEN_VALUE_NEG",
	"TOKEN_VALUE_LSHIFT",
	"TOKEN_VALUE_RSHIFT",
	"TOKEN_VALUE_BITAND_ASSIGN",
	"TOKEN_VALUE_BITOR_ASSIGN",
	"TOKEN_VALUE_XOR_ASSIGN",
	"TOKEN_VALUE_LSHIFT_ASSIGN",
	"TOKEN_VALUE_QUES",
	"TOKEN_VALUE_COLON",
	"TOKEN_VALUE_COMMA",
	"TOKEN_VALUE_DOT",
	"TOKEN_VALUE_SEM",
	"TOKEN_VALUE_LITERAL_INT",
	"TOKEN_VALUE_LITERAL_STRING",
	"TOKEN_VALUE_VOID",
	"TOKEN_VALUE_INT",
	"TOKEN_VALUE_BOOL",
	"TOKEN_VALUE_TRUE",
	"TOKEN_VALUE_FALSE",
	"TOKEN_VALUE_IF",
	"TOKEN_VALUE_ELSE",
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
	"EXPR_TYPE_BOOL",
	"EXPR_TYPE_CALL",
	"EXPR_TYPE_IDENT",
	"EXPR_TYPE_PLUS",
	"EXPR_TYPE_MINUS",
	"EXPR_TYPE_NOT",
	"EXPR_TYPE_ADD",
	"EXPR_TYPE_SUB",
	"EXPR_TYPE_MUL",
	"EXPR_TYPE_DIV",
	"EXPR_TYPE_MOD",
	"EXPR_TYPE_NE",
	"EXPR_TYPE_EQ",
	"EXPR_TYPE_LT",
	"EXPR_TYPE_LE",
	"EXPR_TYPE_GT",
	"EXPR_TYPE_GE",
	"EXPR_TYPE_AND",
	"EXPR_TYPE_OR",
	"EXPR_TYPE_BITAND",
	"EXPR_TYPE_BITOR",
	"EXPR_TYPE_XOR",
	"EXPR_TYPE_NEG",
	"EXPR_TYPE_LSHIFT",
	"EXPR_TYPE_RSHIFT",
	"EXPR_TYPE_COND",
};

static const char* statTypeToString[] =
{
	"STATEMENT_TYPE_EMPTY",
	"STATEMENT_TYPE_DECLARATION",
	"STATEMENT_TYPE_ASSIGN",
	"STATEMENT_TYPE_ADD_ASSIGN",
	"STATEMENT_TYPE_SUB_ASSIGN",
	"STATEMENT_TYPE_MUL_ASSIGN",
	"STATEMENT_TYPE_DIV_ASSIGN",
	"STATEMENT_TYPE_MOD_ASSIGN",
	"STATEMENT_TYPE_BITAND_ASSIGN",
	"STATEMENT_TYPE_BITOR_ASSIGN",
	"STATEMENT_TYPE_XOR_ASSIGN",
	"STATEMENT_TYPE_LSHIFT_ASSIGN",
	"STATEMENT_TYPE_INC",
	"STATEMENT_TYPE_DEC",
	"STATEMENT_TYPE_IF",
	"STATEMENT_TYPE_RETURN",
	"STATEMENT_TYPE_EXPRESSION",
	"STATEMENT_TYPE_COMPOUND",
};

static const char* typeIdToString[] =
{
	"TYPE_INIT",
	"TYPE_VOID",
	"TYPE_INT",
	"TYPE_BOOL",
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
static void _Printer_assignStatement(Printer* p, Statement* stat);
static void _Printer_ifStatement(Printer* p, Statement* stat);
static void _Printer_expression(Printer* p, Expression* expr);
static void _Printer_condtionExpression(Printer* p, Expression* expr);
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

	case STATEMENT_TYPE_ASSIGN:
	case STATEMENT_TYPE_ADD_ASSIGN:
	case STATEMENT_TYPE_SUB_ASSIGN:
	case STATEMENT_TYPE_MUL_ASSIGN:
	case STATEMENT_TYPE_DIV_ASSIGN:
	case STATEMENT_TYPE_MOD_ASSIGN:
	case STATEMENT_TYPE_BITAND_ASSIGN:
	case STATEMENT_TYPE_BITOR_ASSIGN:
	case STATEMENT_TYPE_XOR_ASSIGN:
	case STATEMENT_TYPE_LSHIFT_ASSIGN:
		_Printer_indent(p); printf("assign=\n");
		p->level++;
		_Printer_assignStatement(p, stat);
		p->level--;
		break;

	case STATEMENT_TYPE_INC:
		_Printer_indent(p); printf("incExpr=\n");
		p->level++;
		_Printer_expression(p, stat->incExpr);
		p->level--;
		break;

	case STATEMENT_TYPE_DEC:
		_Printer_indent(p); printf("decExpr=\n");
		p->level++;
		_Printer_expression(p, stat->decExpr);
		p->level--;
		break;

	case STATEMENT_TYPE_IF:
		_Printer_indent(p); printf("ifStat=\n");
		p->level++;
		_Printer_ifStatement(p, stat);
		p->level--;
		break;
		
	}
}

void _Printer_assignStatement(Printer* p, Statement* stat)
{
	_Printer_indent(p); printf("leftExpr=\n");
	{
		p->level++;
		_Printer_expression(p, stat->assign.leftExpr);
		p->level--;
	}
	_Printer_indent(p); printf("rightExpr=\n");
	{
		p->level++;
		_Printer_expression(p, stat->assign.rightExpr);
		p->level--;
	}
}

void _Printer_ifStatement(Printer* p, Statement* stat)
{
	_Printer_indent(p); printf("condition=\n");
	p->level++;
	_Printer_expression(p, stat->ifStat.condition);
	p->level--;

	_Printer_indent(p); printf("statement=\n");
	p->level++;
	_Printer_statement(p, stat->ifStat.statement);
	p->level--;

	_Printer_indent(p); printf("elseStat=%s\n", stat->ifStat.elseStat ? "" : "NULL");
	if (stat->ifStat.elseStat)
	{
		p->level++;
		_Printer_statement(p, stat->ifStat.elseStat);
		p->level--;
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

	case EXPR_TYPE_BOOL:
		_Printer_indent(p); printf("boolExpr=%s\n", expr->boolExpr ? "true" : "false");
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

	case EXPR_TYPE_PLUS:
	case EXPR_TYPE_MINUS:
	case EXPR_TYPE_NOT:
	case EXPR_TYPE_NEG:
		_Printer_indent(p); printf("unaryExpr=\n");
		p->level++;
		_Printer_expression(p, expr->unaryExpr);
		p->level--;
		break;

	case EXPR_TYPE_ADD:
	case EXPR_TYPE_SUB:
	case EXPR_TYPE_MUL:
	case EXPR_TYPE_DIV:
	case EXPR_TYPE_MOD:
	case EXPR_TYPE_NE:
	case EXPR_TYPE_EQ:
	case EXPR_TYPE_LT:
	case EXPR_TYPE_LE:
	case EXPR_TYPE_GT:
	case EXPR_TYPE_GE:
	case EXPR_TYPE_AND:
	case EXPR_TYPE_OR:
	case EXPR_TYPE_BITAND:
	case EXPR_TYPE_BITOR:
	case EXPR_TYPE_XOR:
	case EXPR_TYPE_LSHIFT:
	case EXPR_TYPE_RSHIFT:
		_Printer_indent(p); printf("binaryExpr=\n");
		p->level++;
		_Printer_binaryExpression(p, expr);
		p->level--;
		break;

	case EXPR_TYPE_COND:
		_Printer_indent(p); printf("condExpr=\n");
		p->level++;
		_Printer_condtionExpression(p, expr);
		p->level--;
		break;

	}

}

void _Printer_condtionExpression(Printer* p, Expression* expr)
{
	_Printer_indent(p); printf("expr1=\n");
	{
		p->level++;
		_Printer_expression(p, expr->condExpr.expr1);
		p->level--;
	}

	_Printer_indent(p); printf("expr2=\n");
	{
		p->level++;
		_Printer_expression(p, expr->condExpr.expr2);
		p->level--;
	}

	_Printer_indent(p); printf("expr3=\n");
	{
		p->level++;
		_Printer_expression(p, expr->condExpr.expr3);
		p->level--;
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "generator.h"
#include "message.h"

static void _Generator_variant(Generator* gen, Declaration* decl);
static void _Generator_function(Generator* gen, Declaration* decl);
static void _Generator_parameterList(Generator* gen, Vector params, StringBuffer* buf);
static void _Generator_statement(Generator* gen, Declaration* decl, Statement* stat);
static void _Generator_compoundStatement(Generator* gen, Declaration* decl, Vector block);
static void _Generator_expressionStatement(Generator* gen, Expression* expr);
static void _Generator_returnStatement(Generator* gen, Statement* stat);
static void _Generator_expression(Generator* gen, Expression* expr, StringBuffer* buf);
static void _Generator_callExpression(Generator* gen, Expression* expr, StringBuffer* buf);
static void _Generator_assignExpression(Generator* gen, Expression* expr, StringBuffer* buf);

static void _Generator_indent(Generator* gen, StringBuffer* buf);

void Generator_init(Generator* gen, GenerateTarget target)
{
	StringBuffer_init(&gen->header);
	StringBuffer_init(&gen->srcDecl);
	StringBuffer_init(&gen->srcDef);
	StringBuffer_init(&gen->initGlobal);
	StringBuffer_init(&gen->main);
	gen->module = NULL;
	gen->target = target;
	gen->level = 0;
	gen->inMain = false;
}

void Generator_destroy(Generator* gen)
{
	StringBuffer_destroy(&gen->header);
	StringBuffer_destroy(&gen->srcDecl);
	StringBuffer_destroy(&gen->srcDef);
	StringBuffer_destroy(&gen->initGlobal);
}

void Generator_generate(Generator* gen, Module* module)
{
	gen->module = module;

	for (int i = 0; i < module->declarations.size; i++)
	{
		Declaration* decl = (Declaration*)Vector_get(&module->declarations, i);
		switch (decl->type)
		{
		case DECL_TYPE_VARIANT:
			_Generator_variant(gen, decl);
			break;

		case DECL_TYPE_FUNCTION:
			_Generator_function(gen, decl);
			break;
		}
	}
}

void Generator_getSource(Generator* gen, StringBuffer* source)
{
	StringBuffer_appendString(source, (String*)&gen->srcDecl);
	StringBuffer_appendString(source, (String*)&gen->srcDef);

	StringBuffer_append(source, "int main()\n{\n");
	StringBuffer_appendString(source, (String*)&gen->initGlobal);
	StringBuffer_appendString(source, (String*)&gen->main);
	StringBuffer_append(source, "}\n");
}

void _Generator_variant(Generator* gen, Declaration* decl)
{
	VarDecl* var = &decl->variant;
	if (gen->level == 0 && decl->exported)
	{
		//extern
		StringBuffer_append(&gen->header, "extern ");

		//type
		StringBuffer_appendString(&gen->header, &var->type.name);
		StringBuffer_append(&gen->header, " ");

		//name
		StringBuffer_appendString(&gen->header, &var->name);

		StringBuffer_append(&gen->header, ";\n");
	}

	if (gen->level == 0)
	{
		//static
		if (!decl->exported)
			StringBuffer_append(&gen->srcDecl, "static ");

		//type
		StringBuffer_appendString(&gen->srcDecl, &var->type.name);
		StringBuffer_append(&gen->srcDecl, " ");

		//name
		StringBuffer_appendString(&gen->srcDecl, &var->name);

		if (var->initExpr)
		{
			if (var->initExpr->type == EXPR_TYPE_CALL)
			{
				StringBuffer_append(&gen->initGlobal, "\t");
				StringBuffer_appendString(&gen->initGlobal, &var->name);
				StringBuffer_append(&gen->initGlobal, " = ");
				_Generator_expression(gen, var->initExpr, &gen->initGlobal);
				StringBuffer_append(&gen->initGlobal, ";\n");
			}
			else
			{
				StringBuffer_append(&gen->srcDecl, " = ");
				_Generator_expression(gen, var->initExpr, &gen->srcDecl);
			}
		}

		StringBuffer_append(&gen->srcDecl, ";\n");
		return;
	}

	StringBuffer* buf = gen->inMain ? &gen->main : &gen->srcDef;
	_Generator_indent(gen, buf);

	//type
	StringBuffer_appendString(buf, &var->type.name);
	StringBuffer_append(buf, " ");

	//name
	StringBuffer_appendString(buf, &var->name);

	if (var->initExpr)
	{
		StringBuffer_append(buf, " = ");
		_Generator_expression(gen, var->initExpr, buf);
	}

	StringBuffer_append(buf, ";\n");
}

void _Generator_function(Generator* gen, Declaration* decl)
{
	FuncDecl* func = &decl->function;

	if (String_compare(&func->name, "main") != 0)  //not 'main'
	{
		if (decl->exported)
		{
			//extern
			StringBuffer_append(&gen->header, "extern ");

			//type
			StringBuffer_appendString(&gen->header, &func->resType.name);
			StringBuffer_append(&gen->header, " ");

			//name
			StringBuffer_appendString(&gen->header, &func->name);

			//parameters
			_Generator_parameterList(gen, func->parameters, &gen->header);

			StringBuffer_append(&gen->header, ";\n");
		}
		else
		{
			//static
			StringBuffer_append(&gen->srcDecl, "static ");

			//type
			StringBuffer_appendString(&gen->srcDecl, &func->resType.name);
			StringBuffer_append(&gen->srcDecl, " ");

			//name
			StringBuffer_appendString(&gen->srcDecl, &func->name);

			//parameters
			_Generator_parameterList(gen, func->parameters, &gen->srcDecl);

			StringBuffer_append(&gen->srcDecl, ";\n");
		}

		//type
		StringBuffer_appendString(&gen->srcDef, &func->resType.name);
		StringBuffer_append(&gen->srcDef, " ");

		//name
		StringBuffer_appendString(&gen->srcDef, &func->name);

		//parameters
		_Generator_parameterList(gen, func->parameters, &gen->srcDef);

		StringBuffer_append(&gen->srcDef, " ");

		//block
		_Generator_compoundStatement(gen, decl, func->block);

		return;
	}

	//statements
	gen->inMain = true;
	gen->level++;

	for (int i = 0; i < func->block.size; ++i)
	{
		Statement* subStat = Vector_get(&func->block, i);
		_Generator_statement(gen, decl, subStat);
	}

	gen->level--;
	gen->inMain = false;
}

void _Generator_parameterList(Generator* gen, Vector params, StringBuffer* buf)
{
	StringBuffer_append(buf, "(");

	for (int i = 0; i < params.size; ++i)
	{
		if (i != 0)
			StringBuffer_append(buf, ", ");

		Parameter* p = Vector_get(&params, i);

		//type
		StringBuffer_appendString(buf, &p->type.name);

		StringBuffer_append(buf, " ");

		//name
		StringBuffer_appendString(buf, &p->name);
	}

	StringBuffer_append(buf, ")");
}

void _Generator_statement(Generator* gen, Declaration* decl, Statement* stat)
{
	switch (stat->type)
	{
	case STATEMENT_TYPE_DECLARATION:
		_Generator_variant(gen, &stat->declaration);
		break;

	case STATEMENT_TYPE_COMPOUND:
		_Generator_compoundStatement(gen, decl, stat->compound);
		break;
		
	case STATEMENT_TYPE_EXPRESSION:
		_Generator_expressionStatement(gen, stat->expr);
		break;

	case STATEMENT_TYPE_RETURN:
		_Generator_returnStatement(gen, stat);
		break;
	}
}

void _Generator_compoundStatement(Generator* gen, Declaration* decl, Vector block)
{
	StringBuffer* buf = gen->inMain ? &gen->main : &gen->srcDef;

	_Generator_indent(gen, buf);
	StringBuffer_append(buf, "{\n");

	gen->level++;

	for (int i = 0; i < block.size; ++i)
	{
		Statement* subStat = Vector_get(&block, i);
		_Generator_statement(gen, decl, subStat);
	}

	gen->level--;
	_Generator_indent(gen, buf);
	StringBuffer_append(buf, "}\n");
}

void _Generator_expressionStatement(Generator* gen, Expression* expr)
{
	StringBuffer* buf = gen->inMain ? &gen->main : &gen->srcDef;
	_Generator_indent(gen, buf);

	_Generator_expression(gen, expr, buf);

	StringBuffer_append(buf, ";\n");
}

void _Generator_returnStatement(Generator* gen, Statement* stat)
{
	StringBuffer* buf = gen->inMain ? &gen->main : &gen->srcDef;
	_Generator_indent(gen, buf);

	StringBuffer_append(buf, "return");
	if (stat->returnExpr)
	{
		StringBuffer_append(buf, " ");
		_Generator_expression(gen, stat->returnExpr, buf);
	}

	StringBuffer_append(buf, ";\n");
}

void _Generator_expression(Generator* gen, Expression* expr, StringBuffer* buf)
{
	char tmp[64];
	int len;

	switch (expr->type)
	{
	case EXPR_TYPE_IDENT:
		StringBuffer_appendString(buf, &expr->identExpr);
		break;

	case EXPR_TYPE_INT:
		len = snprintf(tmp, sizeof(tmp), "%d", expr->intExpr);
		StringBuffer_appendN(buf, tmp, len);
		break;

	case EXPR_TYPE_CALL:
		_Generator_callExpression(gen, expr, buf);
		break;

	case EXPR_TYPE_ASSIGN:
		_Generator_assignExpression(gen, expr, buf);
		break;
	}
}

void _Generator_callExpression(Generator* gen, Expression* expr, StringBuffer* buf)
{
	StringBuffer_appendString(buf, &expr->callExpr.func->identExpr);
	StringBuffer_append(buf, "(");

	for (int i = 0; i < expr->callExpr.args.size; ++i)
	{
		if (i != 0)
			StringBuffer_append(buf, ", ");

		Expression* arg = Vector_get(&expr->callExpr.args, i);
		_Generator_expression(gen, arg, buf);
	}

	StringBuffer_append(buf, ")");
}

void _Generator_assignExpression(Generator* gen, Expression* expr, StringBuffer* buf)
{
	_Generator_expression(gen, expr->assignExpr.lvalueExpr, buf);
	StringBuffer_append(buf, " = ");
	_Generator_expression(gen, expr->assignExpr.rvalueExpr, buf);
}

void _Generator_indent(Generator* gen, StringBuffer* buf)
{
	for (int i = 0; i < gen->level; ++i)
		StringBuffer_append(buf, "\t");
}

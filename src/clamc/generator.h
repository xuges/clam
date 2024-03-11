#ifndef CLAM_GENERATOR_H
#define CLAM_GENERATOR_H

#include "ast.h"

enum GenerateTarget
{
	GENERATE_TARGE_C,
};
typedef enum GenerateTarget GenerateTarget;

struct Generator
{
	StringBuffer declarations;
	StringBuffer definitions;
	GenerateTarget target;
	int indentLevel;
};
typedef struct Generator Generator;

void Generator_init(Generator* gen, GenerateTarget target);
void Generator_destroy(Generator* gen);

void Generator_enterDeclaration(Generator* gen, Declaration* decl);
void Generator_leaveDeclaration(Generator* gen, Declaration* decl);
void Generator_enterStatement(Generator* gen, Statement* stat);
void Generator_leaveStatement(Generator* gen, Statement* stat);
void Generator_genPrimaryExpression(Generator* gen, Expression* expr);
void Generator_genCallExpression(Generator* gen, Expression* expr);

#endif
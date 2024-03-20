#ifndef CLAM_GENERATOR_H
#define CLAM_GENERATOR_H

#include "ast.h"
#include "module.h"

enum GenerateTarget
{
	GENERATE_TARGE_C,
};
typedef enum GenerateTarget GenerateTarget;

struct Generator
{
	StringBuffer header;
	StringBuffer srcDecl;
	StringBuffer srcDef;
	StringBuffer initGlobal;
	StringBuffer main;
	
	Module* module;
	GenerateTarget target;
	int level;
	bool inMain;
};
typedef struct Generator Generator;

void Generator_init(Generator* gen, GenerateTarget target);
void Generator_destroy(Generator* gen);

void Generator_generate(Generator* gen, Module* mod);
void Generator_getSource(Generator* gen, StringBuffer* source);

#endif
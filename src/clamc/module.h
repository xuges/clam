#ifndef CLAM_MODULE_H
#define CLAM_MODULE_H

#include "ast.h"
#include "vector.h"

struct Module
{
	Vector declarations;
	Vector functions;
};
typedef struct Module Module;

void Module_init(Module* mod);
void Module_destroy(Module* mod);
void Module_addDeclaration(Module* mod, Declaration* decl);

#endif
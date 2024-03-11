
#include "module.h"

void Module_init(Module* mod)
{
	Vector_init(&mod->declarations, sizeof(Declaration));
	Vector_init(&mod->functions, sizeof(Declaration));
}

void Module_destroy(Module* mod)
{
	Vector_destroy(&mod->declarations);
	Vector_destroy(&mod->functions);
}

void Module_addDeclaration(Module* mod, Declaration* decl)
{
	Vector_add(&mod->declarations, decl);
	if (decl->type == DECL_TYPE_FUNCTION)
		Vector_add(&mod->functions, decl);
}
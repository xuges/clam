#ifndef CLAM_ANALYZER_H
#define CLAM_ANALYZER_H

#include "stack.h"
#include "module.h"
#include "generator.h"

struct Analyzer
{
	Stack stack;  //Stack<Variant>
	Module* module;
	int level;
	bool hasReturn;
};
typedef struct Analyzer Analyzer;

void Analyzer_init(Analyzer* anly);
void Analyzer_destroy(Analyzer* anly);

void Analyzer_analyse(Analyzer* anly, Module* module);



#endif
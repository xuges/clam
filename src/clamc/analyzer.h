#ifndef CLAM_ANALYZER_H
#define CLAM_ANALYZER_H

#include "module.h"
#include "generator.h"

struct Analyzer
{
	Module*   module;
	Generator* gen;
};
typedef struct Analyzer Analyzer;

void Analyzer_init(Analyzer* anly);
void Analyzer_destroy(Analyzer* anly);

void Analyzer_generate(Analyzer* anly, Module* module, Generator* gen);



#endif
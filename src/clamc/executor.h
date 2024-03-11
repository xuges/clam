#ifndef CLAM_EXECUTOR_H
#define CLAM_EXECUTOR_H

#include "module.h"

#include "vector.h"
#include "stack.h"

struct Executor
{
	Stack stack;
	Module* module;
};
typedef struct Executor Executor;

void Executor_init(Executor* exec);
void Executor_destroy(Executor* exec);
void Executor_run(Executor* exec, Module* module);

#endif
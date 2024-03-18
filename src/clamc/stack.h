#ifndef CLAM_STACK_H
#define CLAM_STACK_H

#include "vector.h"

typedef Vector Stack;

void Stack_init(Stack* stack, int elemSize);
void Stack_destroy(Stack* stack);

void Stack_reserve(Stack* stack, int cap);

void Stack_push(Stack* stack, void* elem);
void* Stack_pop(Stack* stack);
void* Stack_top(Stack* stack);


#endif
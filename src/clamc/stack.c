#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "message.h"


void Stack_init(Stack* stack, int elemSize)
{
	Vector_init(stack, elemSize);
}

void Stack_destroy(Stack* stack)
{
	Vector_destroy(stack);
}

void Stack_reserve(Stack* stack, int cap)
{
	Vector_reserve(stack, cap);
}

void Stack_push(Stack* stack, void* elem)
{
	Vector_add(stack, elem);
}

void* Stack_pop(Stack* stack)
{
	void* elem = Vector_get(stack, stack->size - 1);
	Vector_resize(stack, stack->size - 1);
	return elem;
}

void* Stack_top(Stack* stack)
{
	return Vector_get(stack, stack->size - 1);
}

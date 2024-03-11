#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "message.h"


void Stack_init(Stack* stack, int elemSize)
{
	stack->data = NULL;
	stack->cap = 0;
	stack->size = 0;
	stack->elemSize = elemSize;
}

void Stack_destroy(Stack* stack)
{
	if (stack->data)
	{
		free(stack->data);
		stack->size = 0;
		stack->cap = 0;
		stack->data = NULL;
	}
}

void Stack_reserve(Stack* stack, int cap)
{
	if (cap < 1)
		cap = 1;

	if (stack->cap < cap)
	{
		stack->data = realloc(stack->data, stack->elemSize * cap);
		if (!stack->data)
			error(NULL, 0, 0, "out of memory");
		stack->cap = cap;
	}
}

void Stack_push(Stack* stack, void* elem)
{
	//grow
	if (stack->size + 1 > stack->cap)
		Stack_reserve(stack, stack->size * 2);

	memcpy(stack->data + stack->size * stack->elemSize, elem, stack->elemSize);
	stack->size++;
}

void* Stack_pop(Stack* stack)
{
	void* elem = Stack_top(stack);
	stack->size--;
	return elem;
}

void* Stack_top(Stack* stack)
{
	return stack->data + (stack->size - 1) * stack->elemSize;
}

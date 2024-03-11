#ifndef CLAM_STACK_H
#define CLAM_STACK_H

struct Stack
{
	char* data;
	int size;
	int cap;
	int elemSize;
};
typedef struct Stack Stack;

void Stack_init(Stack* stack, int elemSize);
void Stack_destroy(Stack* stack);

void Stack_reserve(Stack* stack, int cap);

void Stack_push(Stack* stack, void* elem);
void* Stack_pop(Stack* stack);
void* Stack_top(Stack* stack);


#endif
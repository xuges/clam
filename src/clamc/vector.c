#include <stdlib.h>
#include <string.h>

#include "vector.h"
#include "message.h"

void Vector_init(Vector* vec, int elemSize)
{
	vec->data = NULL;
	vec->cap = 0;
	vec->size = 0;
	vec->elemSize = elemSize;
}

void Vector_destroy(Vector* vec)
{
	if (vec->data)
	{
		free(vec->data);
		vec->data = NULL;
		vec->size = 0;
		vec->cap = 0;
	}

}

void Vector_reserve(Vector* vec, int cap)
{
	if (cap < 1)
		cap = 1;

	if (vec->cap < cap)
	{
		vec->data = realloc(vec->data, vec->elemSize * cap);
		if (!vec->data)
			error(NULL, 0, 0, "out of memory");
		vec->cap = cap;
	}
}

void Vector_resize(Vector* vec, int size)
{
	if (size > vec->cap)
		Vector_reserve(vec, size);
	vec->size = size;
}

void Vector_add(Vector* vec, void* elem)
{
	//grow
	if (vec->size + 1 > vec->cap)
		Vector_reserve(vec, vec->size * 2);

	memcpy((char*)vec->data + vec->size * vec->elemSize, elem, vec->elemSize);
	vec->size++;
}

void* Vector_get(Vector* vec, int index)
{
	return (char*)vec->data + index * vec->elemSize;
}

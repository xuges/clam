#ifndef CLAM_VECTOR_H
#define CLAM_VECTOR_H

struct Vector
{
	void* data;
	int size;
	int cap;
	int elemSize;
};
typedef struct Vector Vector;

void Vector_init(Vector* vec, int elemSize);
void Vector_destroy(Vector* vec);
void Vector_reserve(Vector* vec, int cap);
void Vector_resize(Vector* vec, int size);
void Vector_add(Vector* vec, void* elem);
void* Vector_get(Vector* vec, int index);

#endif
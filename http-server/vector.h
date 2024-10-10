#ifndef VECTOR_HEADER_
#define VECTOR_HEADER_

#include <cstdlib>
#include <cstring>
#include "vector.h"

template<typename T>
struct vector_t {
	T* buf;
	int len;
	int capacity;
};


// TODO: test all of this
template<typename T>
vector_t<T>* newVector(int capacity) {
	vector_t<T>* vec = (vector_t<T>*) malloc(sizeof(vector_t<T>));

	vec->buf = (T*) malloc(capacity * sizeof(T));
	vec->len = 0;
	vec->capacity = capacity;

	return vec;
}

template<typename T>
void push(vector_t<T> *vec, T item) {
	if (vec->len >= vec->capacity) {
		vec->capacity *= 2;
		T* newBuf = (T*) malloc(vec->capacity * sizeof(T));
		memcpy(newBuf, vec->buf, vec->len);

		free(vec->buf);
		vec->buf = newBuf;
	}

	vec->buf[vec->len - 1] = item;
	vec->len++;
}

template<typename T>
void reset(vector_t<T> *vec) {
	vec->len = 0;
}

template<typename T>
void pushMany(vector_t<T> *vec, T* items, int size) {
	int expectedSize = vec->len + size;
	if (expectedSize >= vec->capacity) {
		if (vec->capacity * 2 > expectedSize) {
			vec->capacity *= 2;
		} else {
			vec->capacity = expectedSize;
		}

		T* newBuf = (T*) malloc(vec->capacity * sizeof(T));
		memcpy(newBuf, vec->buf, vec->len);
		free(vec->buf);
		vec->buf = newBuf;
	}

	memcpy(vec->buf + vec->len, items, size);
	vec->len += size;
}

#endif

// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <new>
#include <stdio.h>

#include "overloaded.h"
#include "nedmalloc.h"


void *operator new(size_t size) throw(std::bad_alloc) {
	void *p = malloc(size);

	if (!p) {
		throw std::bad_alloc();
	}

	return p;
}

void operator delete(void *p) throw() {
	free(p);
}

void *operator new(size_t size, const std::nothrow_t &) throw() {
	return malloc(size);
}

void operator delete(void *p, const std::nothrow_t &) throw() {
	free(p);
}

void *operator new[](size_t size) throw(std::bad_alloc) {
	void *p = malloc(size);

	if (!p) {
		throw std::bad_alloc();
	}

	return p;
}

void operator delete[](void *p) throw() {
	free(p);
}

void *operator new[](size_t size, const std::nothrow_t &) throw() {
	return malloc(size);
}

void operator delete[](void *p, const std::nothrow_t &) throw() {
	free(p);
}

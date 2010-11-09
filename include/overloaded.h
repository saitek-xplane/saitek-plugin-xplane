// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __OVERLOADED_H
#define __OVERLOADED_H

#include <new>

void *operator new(size_t size) throw(std::bad_alloc);
void operator delete(void *p) throw();

void *operator new(size_t size, const std::nothrow_t &) throw();
void operator delete(void *p, const std::nothrow_t &) throw();

void *operator new[](size_t size) throw(std::bad_alloc);
void operator delete[](void *p) throw();

void *operator new[](size_t size, const std::nothrow_t &) throw();
void operator delete[](void *p, const std::nothrow_t &) throw();

#endif

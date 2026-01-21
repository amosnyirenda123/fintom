// include/fa/fa.h
#ifndef ITER_H
#define ITER_H

#include <stdbool.h>
#include <stddef.h>

// Iteration support
typedef struct {
    void* obj;
    size_t index;
} Iter;


Iter iter_create(void* obj, size_t obj_size);
bool iter_has_next(const Iter* iter);
void* iter_next(Iter* iter);
void* iter_current(Iter* iter);
void iter_reset(Iter* iter);


#endif //ITER_H
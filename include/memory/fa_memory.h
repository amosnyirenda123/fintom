#ifndef FA_MEMORY_H
#define FA_MEMORY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void* fa_malloc(size_t size);
void* fa_calloc(size_t nmemb, size_t size);
void* fa_realloc(void* ptr, size_t size);
void  fa_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* FA_MEMORY_H */

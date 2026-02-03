#include "../include/fa_error_utils.h"
#include "../include/memory/fa_memory.h"
#include <stdlib.h>

void* fa_malloc(size_t size)
{
    return malloc(size);
}

void* fa_calloc(size_t nmemb, size_t size)
{
    return calloc(nmemb, size);
}

void* fa_realloc(void* ptr, size_t size)
{
    return realloc(ptr, size);
}

void fa_free(void* ptr)
{
    free(ptr);
}


fa_error_t fa_alloc_ptr(void** out, size_t size)
{
    if (!out) 
        return FA_ERR_NULL_ARGUMENT;

    *out = malloc(size);
    if (!*out)
        return FA_ERR_OUT_OF_MEMORY;

    return FA_SUCCESS;
}

fa_error_t fa_calloc_ptr(void** out, size_t nmemb, size_t size)
{
    if (!out)
        return FA_ERR_NULL_ARGUMENT;

    *out = calloc(nmemb, size);
    if (!*out)
        return FA_ERR_OUT_OF_MEMORY;

    return FA_SUCCESS;
}

fa_error_t fa_realloc_ptr(void** out, size_t size)
{
    if (!out)
        return FA_ERR_NULL_ARGUMENT;

    void* tmp = realloc(*out, size);
    if (!tmp)
        return FA_ERR_OUT_OF_MEMORY;

    *out = tmp;
    return FA_SUCCESS;
}


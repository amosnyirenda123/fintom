#ifndef FA_ERROR_UTILS_H
#define FA_ERROR_UTILS_H

#include "fa_error.h"


/* Macro to check if error belongs to a specific module */
#define FA_IS_FA_ERROR(err) ((err) >= 200 && (err) <= 299)
#define FA_IS_IO_ERROR(err) ((err) >= 400 && (err) <= 499)
#define FA_IS_PDA_ERROR(err) ((err) >= 600 && (err) <= 699)
#define FA_IS_TM_ERROR(err) ((err) >= 700 && (err) <= 799)

/* Macro for quick error checking */
#define FA_CHECK(expr) do { \
    fa_error_t __err = (expr); \
    if (__err != FA_SUCCESS) return __err; \
} while(0)

#define FA_CHECK_MSG(expr, msg) do { \
    fa_error_t __err = (expr); \
    if (__err != FA_SUCCESS) { \
        fprintf(stderr, "%s: %s\n", msg, fa_error_str(__err)); \
        return __err; \
    } \
} while(0)


#define FA_RETURN_IF_ERROR(expr)        \
    do {                                \
        fa_error_t _err = (expr);       \
        if (_err != FA_SUCCESS)         \
            return _err;                \
    } while (0)

#define FA_RETURN_IF_NULL(ptr)          \
    do {                                \
        if (!(ptr))                     \
            return FA_ERROR_NULL_ARGUMENT; \
    } while (0)

#ifdef FA_DEBUG
#include <stdio.h>
#include <stdlib.h>
#define FA_ASSERT(cond)                                     \
    do {                                                    \
        if (!(cond)) {                                     \
            fprintf(stderr,                                \
                "FA ASSERT FAILED (%s:%d): %s\n",          \
                __FILE__, __LINE__, #cond);                 \
            abort();                                        \
        }                                                   \
    } while (0)
#else
#define FA_ASSERT(cond) ((void)0)
#endif

#endif /* FA_ERROR_UTILS_H */

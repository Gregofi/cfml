#include <string.h>

#include "include/constant.h"
#include "include/memory.h"

void init_constant_pool(constant_pool_t* pool) {
    memset(pool, 0, sizeof(*pool));
}

size_t add_constant(constant_pool_t* pool, constant_t constant) {
    if (pool->len >= pool->capacity) {
        pool->capacity = NEW_CAPACITY(pool->capacity);
        pool->data = (constant_t*)realloc(pool->data, pool->capacity * sizeof(*(pool->data))); 
    }

    pool->data[pool->len++] = constant;
    return pool->len - 1;
}

void free_constant_pool(constant_pool_t* pool) {
    free(pool->data);
    init_constant_pool(pool);
}

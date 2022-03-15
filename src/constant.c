#include <string.h>

#include "include/constant.h"
#include "include/memory.h"

void init_constant_pool(constant_pool_t* pool) {
    memset(pool, 0, sizeof(*pool));
}

size_t add_constant(constant_pool_t* pool, value_t constant) {
    if (pool->len >= pool->capacity) {
        pool->capacity = NEW_CAPACITY(pool->capacity);
        pool->data = (value_t*)realloc(pool->data, pool->capacity * sizeof(*(pool->data))); 
    }

    pool->data[pool->len++] = constant;
    return pool->len - 1;
}

void free_constant_pool(constant_pool_t* pool) {
    for(size_t i = 0; i < pool->len; ++ i) {
        if (pool->data[i].type != TYPE_OBJECT)
            continue;

        free(AS_STRING(pool->data[i]));
    }
    free(pool->data);
    init_constant_pool(pool);
}

obj_string_t* build_obj_string(size_t len, const char* ptr) {
    obj_string_t* new_string = (obj_string_t*)malloc(sizeof(*new_string) 
                                    + len + 1);
    new_string->length = len;
    strncpy(new_string->data, ptr, len);
    new_string->data[len] = '\0';
    new_string->obj = (obj_t){.type = OBJ_STRING};
    new_string->length = len;
    return new_string;
}

/// Allocates function object on heap and returns pointer to it, 
/// fields of function are zero initialized
/// with exception of object, which is initialized correctly.
obj_function_t* build_obj_fun() {
    obj_function_t* fun = (obj_function_t*)malloc(sizeof(*fun));
    memset(fun, 0, sizeof(*fun));
    fun->obj = (obj_t){.type = OBJ_FUNCTION};
    return fun;
}

bool is_obj_type(value_t val, obj_type_t type) {
    return IS_OBJ(val) && AS_OBJ(val)->type == type;
}

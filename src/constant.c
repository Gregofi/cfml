#include <string.h>

#include "include/constant.h"
#include "include/objects.h"
#include "include/memory.h"

void init_globals(global_indexes_t* globals) {
    memset(globals, 0, sizeof(*globals));
}

void write_global(global_indexes_t* globals, uint16_t index) {
    if (globals->length >= globals->capacity) {
        globals->capacity = NEW_CAPACITY(globals->capacity);
        globals->indexes = realloc(globals->indexes, globals->capacity);
    }

    globals->indexes[globals->length ++] = index;
}

void free_globals(global_indexes_t* globals) {
    free(globals->indexes);
    init_globals(globals);
}

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

/// Returns new dynamically allocated instance of obj_string_t
obj_string_t* build_obj_string(size_t len, const char* ptr, uint32_t hash) {
    obj_string_t* new_string = (obj_string_t*)malloc(sizeof(*new_string) 
                                    + len + 1);
    new_string->length = len;
    strncpy(new_string->data, ptr, len);
    new_string->data[len] = '\0';
    new_string->obj = (obj_t){.type = OBJ_STRING};
    new_string->length = len;
    new_string->hash = hash;
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

obj_slot_t* build_obj_slot(uint16_t index) {
    obj_slot_t* slot = malloc(sizeof(*slot));
    slot->index = index;
    slot->obj = (obj_t){.type = OBJ_SLOT};
    return slot;
}

bool is_obj_type(value_t val, obj_type_t type) {
    return IS_OBJ(val) && AS_OBJ(val)->type == type;
}

obj_array_t* build_obj_array(size_t size, value_t init) {
    obj_array_t* obj = malloc(sizeof(*obj) + size * sizeof(*obj->values));
    obj->size = size;
    obj->obj = (obj_t){.type = OBJ_ARRAY};
    for (size_t i = 0; i < size; ++ i) {
        obj->values[i] = init;
    }
    return obj;
}

obj_class_t* build_obj_class() {
    
    obj_class_t* obj = malloc(sizeof(*obj));
    obj->obj = (obj_t){.type = OBJ_CLASS};
    obj->size = 0;
    init_hash_map(&obj->methods);
    return obj;
}

obj_instance_t* build_obj_instance(obj_class_t* class, hash_map_t fields, value_t extends) {
    obj_instance_t* obj = malloc(sizeof(*obj));
    obj->extends = extends;
    obj->obj = (obj_t){.type = OBJ_INSTANCE};
    obj->class = class;
    obj->fields = fields;
    return obj;
}

obj_native_fun_t* build_obj_native(native_fun_t fun) {
    obj_native_fun_t* obj = malloc(sizeof(*obj));
    obj->obj = (obj_t){.type = OBJ_NATIVE};
    obj->fun = fun;
    return obj;
}

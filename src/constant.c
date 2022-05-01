#include <string.h>

#include "include/constant.h"
#include "include/objects.h"
#include "include/memory.h"
#include "include/buddy_alloc.h"
#include "include/vm.h"

void init_globals(global_indexes_t* globals) {
    memset(globals, 0, sizeof(*globals));
}

void write_global(global_indexes_t* globals, uint16_t index) {
    if (globals->length >= globals->capacity) {
        globals->capacity = NEW_CAPACITY(globals->capacity);
        globals->indexes = heap_realloc(globals->indexes, globals->capacity * sizeof(*globals->indexes));
        if (!globals->indexes) {
            fprintf(stderr, "Reallocation failed.\n");
            exit(1);
        }
    }

    globals->indexes[globals->length ++] = index;
}

void free_globals(global_indexes_t* globals) {
    heap_free(globals->indexes);
    init_globals(globals);
}

void init_constant_pool(constant_pool_t* pool) {
    memset(pool, 0, sizeof(*pool));
}

size_t add_constant(constant_pool_t* pool, value_t constant) {
    if (pool->len >= pool->capacity) {
        pool->capacity = NEW_CAPACITY(pool->capacity);
        pool->data = (value_t*)heap_realloc(pool->data, pool->capacity * sizeof(*(pool->data)));
    }

    pool->data[pool->len++] = constant;
    return pool->len - 1;
}

void free_constant_pool(constant_pool_t* pool) {
    for(size_t i = 0; i < pool->len; ++ i) {
        if (pool->data[i].type != TYPE_OBJECT)
            continue;
        heap_free(AS_STRING(pool->data[i]));
    }
    heap_free(pool->data);
    init_constant_pool(pool);
}

static obj_t* allocate_obj(size_t size, obj_type_t type, vm_t* vm) {
    obj_t* obj = alloc_with_gc(size, vm);
    obj->type = type;
    // GC: Object is not marked at init
    obj->marked = false;
    // Add to object list
    obj->next = vm->objects;
    vm->objects = obj;
    return obj;
}

/// Returns new dynamically allocated instance of obj_string_t.
obj_string_t* build_obj_string(size_t len, const char* ptr, uint32_t hash, vm_t* vm) {
    obj_string_t* new_string = (obj_string_t*)allocate_obj(sizeof(*new_string) + len + 1, OBJ_STRING, vm);
    new_string->length = len;
    strncpy(new_string->data, ptr, len);
    new_string->data[len] = '\0';
    new_string->length = len;
    new_string->hash = hash;
    return new_string;
}

/// Allocates function object on heap and returns pointer to it,
/// fields of function are zero initialized
/// with exception of object, which is initialized correctly.
obj_function_t* build_obj_fun(vm_t* vm) {
    obj_function_t* fun = (obj_function_t*)allocate_obj(sizeof(*fun), OBJ_FUNCTION, vm);
    fun->args = 0;
    fun->entry_point = 0;
    fun->length = 0;
    fun->locals = 0;
    fun->name = 0;
    return fun;
}

obj_slot_t* build_obj_slot(uint16_t index, vm_t* vm) {
    obj_slot_t* slot = (obj_slot_t*)allocate_obj(sizeof(*slot), OBJ_SLOT, vm);
    slot->index = index;
    return slot;
}

bool is_obj_type(value_t val, obj_type_t type) {
    return IS_OBJ(val) && AS_OBJ(val)->type == type;
}

obj_array_t* build_obj_array(size_t size, value_t init, vm_t* vm) {
    obj_array_t* obj = (obj_array_t*)allocate_obj(sizeof(*obj) + size * sizeof(*obj->values), OBJ_ARRAY, vm);
    obj->size = size;
    for (size_t i = 0; i < size; ++ i) {
        obj->values[i] = init;
    }
    return obj;
}

obj_class_t* build_obj_class(vm_t* vm) {
    obj_class_t* obj = (obj_class_t*)allocate_obj(sizeof(*obj), OBJ_CLASS, vm);
    obj->size = 0;
    init_hash_map(&obj->methods);
    return obj;
}

obj_instance_t* build_obj_instance(obj_class_t* class, hash_map_t fields, value_t extends, vm_t* vm) {
    obj_instance_t* obj = (obj_instance_t*)allocate_obj(sizeof(*obj), OBJ_INSTANCE, vm);
    obj->extends = extends;
    obj->class = class;
    obj->fields = fields;
    return obj;
}

obj_native_fun_t* build_obj_native(native_fun_t fun, vm_t* vm) {
    obj_native_fun_t* obj = (obj_native_fun_t*)allocate_obj(sizeof(*obj), OBJ_NATIVE, vm);
    obj->fun = fun;
    return obj;
}

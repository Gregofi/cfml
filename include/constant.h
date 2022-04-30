#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

typedef struct vm vm_t;

typedef enum {
    TYPE_INTEGER,
    TYPE_NULL,
    TYPE_BOOLEAN,
    TYPE_OBJECT,
} constant_type_t;

typedef enum {
    OBJ_STRING,
    OBJ_ARRAY,
    OBJ_CLASS,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_INSTANCE,
    OBJ_SLOT,
} obj_type_t;

// ------------- OBJECTS --------------

typedef struct obj obj_t;

typedef struct {
    constant_type_t type;
    union {
        int num;
        bool b;
        struct obj* obj;
    };
} value_t;

typedef struct obj {
    obj_type_t type;
    // Used for GC
    bool marked;
    struct obj *next;
} obj_t;

typedef struct {
    obj_t obj;
    size_t length;
    uint32_t hash;
    char data[];
} obj_string_t;

typedef struct {
    obj_t obj;
    // Index to constant pool containing string with func name.
    uint16_t name;
    // Number of arguments.
    uint8_t args;
    // Number of local variables.
    uint16_t locals;
    // Byte where the function starts.
    uint32_t entry_point;
    // Length of the function in bytes.
    size_t length;
} obj_function_t;

typedef struct {
    obj_t obj;
    uint16_t index;
} obj_slot_t;

typedef struct {
    obj_t obj;
    size_t size;
    value_t values[];
} obj_array_t;

typedef value_t (*native_fun_t)(int argCount, value_t* args);

typedef struct {
    obj_t obj;
    native_fun_t fun;
} obj_native_fun_t;

// -------------------------------------

typedef struct {
    size_t capacity;
    size_t length;
    uint16_t* indexes;
} global_indexes_t;

void init_globals(global_indexes_t* globals);
void write_global(global_indexes_t* globals, uint16_t index);
void free_globals(global_indexes_t* globals);

static obj_t* allocate_obj(size_t size, obj_type_t type, vm_t* vm);

obj_string_t* build_obj_string(size_t len, const char* ptr, uint32_t hash, vm_t* vm);

/// Allocates function object on heap and returns pointer to it, fields of function are zero initialized
/// with exception of object, which is initialized correctly.
obj_function_t* build_obj_fun(vm_t* vm);

obj_slot_t* build_obj_slot(uint16_t index, vm_t* vm);

obj_array_t* build_obj_array(size_t size, value_t init, vm_t* vm);

obj_native_fun_t* build_obj_native(native_fun_t fun, vm_t* vm);

// 'Constructor' functions for values.
#define INTEGER_VAL(value) ((value_t){TYPE_INTEGER, {.num = (value)}})
#define BOOL_VAL(value)    ((value_t){TYPE_BOOLEAN, {.b = (value)}})
#define NULL_VAL           ((value_t){TYPE_NULL, {.num = 0}})
#define OBJ_VAL(value)     ((value_t){TYPE_OBJECT, {.obj = (obj_t*)(value)}})

/// Creates an object value containing string.
/// @param len - Length of the string to copy.
/// @param source - const char* pointer to the string to copy from.
#define OBJ_STRING_VAL(len, source, hash, vm) (OBJ_VAL((build_obj_string((len), (source), (hash), (vm)))))
#define OBJ_FUN_VAL(vm) (OBJ_VAL((build_obj_fun(vm))))
#define OBJ_SLOT_VAL(index, vm) OBJ_VAL(build_obj_slot(index, vm))
#define OBJ_ARRAY_VAL(size, init, vm) OBJ_VAL(build_obj_array((size), (init), (vm)))

#define IS_NUMBER(value) ((value).type == TYPE_INTEGER)
#define IS_BOOL(value) ((value).type == TYPE_BOOLEAN)
#define IS_NULL(value) ((value).type == TYPE_NULL)
#define IS_OBJ(value) ((value).type == TYPE_OBJECT)

#define AS_NUMBER(value) ((value).num)
#define AS_BOOL(value) ((value).b)
#define AS_OBJ(value) ((value).obj)

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) is_obj_type(value, OBJ_STRING)
#define AS_STRING(value) (((obj_string_t*)AS_OBJ(value)))
#define AS_CSTRING(value) (((obj_string_t*)AS_OBJ(value))->data)

#define IS_FUNCTION(value) (is_obj_type(value, OBJ_FUNCTION))
#define AS_FUNCTION(value) (((obj_function_t*)AS_OBJ(value)))

#define IS_SLOT(value) (is_obj_type((value), OBJ_SLOT))
#define AS_SLOT(value) (((obj_slot_t*)AS_OBJ(value)))

#define IS_ARRAY(value) (is_obj_type((value), OBJ_ARRAY))
#define AS_ARRAY(value) (((obj_array_t*)AS_OBJ(value)))

#define IS_NATIVE(value) (is_obj_type((value), OBJ_NATIVE))
#define AS_NATIVE(value) (((obj_native_fun_t*)AS_OBJ(value)))

#define IS_FALSY(value) ( (IS_BOOL(value) && !AS_BOOL(value)) || IS_NULL(value) )

bool is_obj_type(value_t val, obj_type_t type);

typedef struct {
    value_t* data;
    size_t capacity;
    size_t len;
} constant_pool_t;

/// Initalizes constant_pool_t with zero values.
void init_constant_pool(constant_pool_t* pool);
void free_constant_pool(constant_pool_t* pool);

/// Adds contant to constant array and returns index to it.
size_t add_constant(constant_pool_t* pool, value_t constant);

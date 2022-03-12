#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// typedef enum {
//     C_INTEGER = 0x00,
//     C_NULL = 0x01,
//     C_STRING = 0x02,
//     C_METHOD = 0x03,
//     C_SLOT = 0x04,
//     C_CLASS = 0x05,
//     C_BOOLEAN = 0x06,
// } constant_serialization_type_t;


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
} obj_type_t;

// ------------- OBJECTS --------------

typedef struct {
    obj_type_t type;
} obj_t;

typedef struct {
    obj_t obj;
    size_t length;
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

// -------------------------------------

typedef struct {
    constant_type_t type;
    union {
        int num;
        bool b;
        obj_t* obj;
    };
} constant_t;

obj_string_t* build_obj_string(size_t len, const char* ptr) {
    obj_string_t* new_string = (obj_string_t*)malloc(sizeof(*new_string) 
                                    + len * sizeof(char) + 1);
    strncpy(new_string->data, ptr, len);
    new_string->data[new_string->length + 1] = '\0';
    obj_t t = {.type = OBJ_STRING};
    new_string->obj = t;
    new_string->length = len;
    return new_string;
}

/// Allocates function object on heap and returns pointer to it, fields of function are zero initialized
/// with exception of object, which is initialized correctly.
obj_function_t* build_obj_fun() {
    obj_function_t* fun = (obj_function_t*)malloc(sizeof(*fun));
    memset(fun, 0, sizeof(*fun));
    fun->obj = (obj_t){.type = OBJ_FUNCTION};
    return fun;
}

// 'Constructor' functions for values.
#define INTEGER_VAL(value) ((constant_t){TYPE_INTEGER, {.num = (value)}})
#define BOOL_VAL(value)    ((constant_t){TYPE_BOOLEAN, {.b = (value)}})
#define NULL_VAL           ((constant_t){TYPE_NULL, {.num = 0}})
#define OBJ_VAL(value)     ((constant_t){TYPE_OBJECT, {.obj = (obj_t*)(value)}})

/// Creates an object value containing string.
/// @param len - Length of the string to copy.
/// @param source - const char* pointer to the string to copy from.
#define OBJ_STRING_VAL(len, source) (OBJ_VAL((build_obj_string((len), (source)))))
#define OBJ_FUN_VAL() (OBJ_VAL((build_obj_fun())))

#define IS_NUMBER(value) ((value).type == TYPE_INTEGER)
#define IS_BOOL(value) ((value).type == TYPE_BOOL)
#define IS_NULL(value) ((value).type == TYPE_NULL)
#define IS_OBJ(value) ((value).type == TYPE_OBJECT)

#define AS_NUMBER(value) ((value).num)
#define AS_BOOL(value) ((value).b)
#define AS_OBJ(value) ((value).obj)

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING);
#define AS_STRING(value) (((obj_string_t*)AS_OBJ(value)))
#define AS_CSTRING(value) (((obj_string_t*)AS_OBJ(value))->data)

#define IS_FUNCTION(value) isObjType(value, OBJ_FUN)
#define AS_FUNCTION(value) (((obj_function_t*)AS_OBJ(value)))

static inline is_obj_type(constant_t val, obj_type_t type) {
    return IS_OBJ(val) && AS_OBJ(val)->type == type;
}

typedef struct {
    constant_t* data;
    size_t capacity;
    size_t len;
} constant_pool_t;

/// Initalizes constant_pool_t with zero values.
void init_constant_pool(constant_pool_t* pool);

/// Adds contant to constant array and returns index to it.
size_t add_constant(constant_pool_t* pool, constant_t constant);


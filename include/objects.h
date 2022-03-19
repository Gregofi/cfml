#pragma once

#include "include/constant.h"
#include "include/hashmap.h"
#define MAX_FIELDS 256

typedef struct {
    obj_t obj;
    uint16_t size;
    hash_map_t methods;
    obj_string_t* fields[MAX_FIELDS];
} obj_class_t;

typedef struct {
    obj_t obj;
    value_t extends;
    obj_class_t* class;
    hash_map_t fields;
} obj_instance_t;

#define OBJ_CLASS_VAL() (OBJ_VAL(build_obj_class()))
obj_class_t* build_obj_class();

#define IS_CLASS(value) (is_obj_type((value), OBJ_CLASS))
#define AS_CLASS(value) (((obj_class_t*)AS_OBJ(value)))

#define IS_INSTANCE(value) (is_obj_type((value), OBJ_INSTANCE))
#define AS_INSTANCE(value) (((obj_instance_t*)AS_OBJ(value)))

#define OBJ_INSTANCE_VAL(class, fields, extends) (OBJ_VAL(build_obj_instance((class), (fields), (extends))))
obj_instance_t* build_obj_instance(obj_class_t* class, hash_map_t fields, value_t extends);

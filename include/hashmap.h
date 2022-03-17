#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "include/constant.h"
#include "include/memory.h"

#define HASH_MAP_LOAD_BALACE 0.8

typedef struct {
    obj_string_t* key;
    value_t value;
} entry_t;

typedef struct {
    size_t count;
    size_t capacity;
    entry_t* entries;
} hash_map_t;

void init_hash_map(hash_map_t* hm);

void free_hash_map(hash_map_t* hm);

/// djb2 hash function, taken from http://www.cse.yorku.ca/~oz/hash.html.
unsigned long hash_string(const char *str);

bool hash_map_insert(hash_map_t* hm, obj_string_t* key, value_t value);

bool hash_map_fetch(hash_map_t* hm, obj_string_t* key, value_t* value);

bool hash_map_delete(hash_map_t* hm, obj_string_t* key);

bool hash_map_update(hash_map_t* hm, obj_string_t* key, value_t new_val);

#pragma once
#include "stdlib.h"

#define ARRAY_GROW_FACTOR 2
#define INIT_ARRAY_SIZE 8
#define NEW_CAPACITY(cap) ((cap) == 0 ? (INIT_ARRAY_SIZE) : (cap) * (ARRAY_GROW_FACTOR))

// Reads bytes from pointer in little endian
#define READ_2BYTES(value) ( *((uint8_t*)(value)) | *((uint8_t*)(value) + 1) << 8)
#define READ_4BYTES(value) ( *(uint8_t*)(value) | ((*(uint8_t*)(value + 1)) << 8) \
            | ((*(uint8_t*)(value + 2)) << 16) | ((*(uint8_t*)(value + 3)) << 24))
#define READ_BYTE(value) (*((uint8_t*)((value))))

#define NOT_IMPLEMENTED() do { fprintf(stderr, "Runtime error: Not implemented: %s:%d.\n", __FILE__, __LINE__); exit(-1);} while(0)

typedef struct {
    size_t first;
    size_t second;
} size_t_pair_t;

#define GREEN_COLOR_TERMINAL "\033[0;32m"
#define RED_COLOR_TERMINAL "\033[0;31m"
#define CLEAR_COLOR_TERMINAL "\033[0;0m"

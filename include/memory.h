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

// Reads one utf8 char and returns number of bytes read, 0 means error.
// size_t read_utf8(uint8_t *src, uint8_t *dest) {
//     if ()
// }

#define NOT_IMPLEMENTED() do { fprintf(stderr, "Not implemented: %s:%d.\n", __FILE__, __LINE__); exit(-1);} while(0)

typedef struct {
    size_t first;
    size_t second;
} size_t_pair_t;

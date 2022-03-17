#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "constant.h"


typedef enum {
    OP_LITERAL = 0x01,
    OP_GET_LOCAL = 0x0A,
    OP_SET_LOCAL = 0x09,
    OP_GET_GLOBAL = 0x0C,
    OP_SET_GLOBAL = 0x0B,
    OP_CALL_FUNCTION = 0x08,
    OP_RETURN = 0x0F,
    OP_LABEL = 0x00,
    OP_JUMP = 0x0E,
    OP_BRANCH = 0x0D,
    OP_PRINT = 0x02,
    OP_ARRAY = 0x03,
    OP_OBJECT = 0x04,
    OP_GET_FIELD = 0x05,
    OP_SET_FIELD = 0x06,
    OP_CALL_METHOD = 0x07,
    OP_DROP = 0x10,
} opcode_t;

typedef struct {
    uint8_t* bytecode;
    size_t size;
    size_t capacity;
    size_t offset;
    constant_pool_t pool;
    global_indexes_t globals;
} chunk_t;

void init_chunk(chunk_t* chunk);
void write_chunk(chunk_t *chunk, uint8_t data);
void free_chunk(chunk_t *chunk);

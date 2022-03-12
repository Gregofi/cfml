#include <string.h>

#include "include/bytecode.h"
#include "include/constant.h"
#include "include/memory.h"


void init_chunk(chunk_t *chunk) {
    memset(chunk, 0, sizeof(*chunk));
    init_constant_pool(&chunk->pool);
}

void write_chunk(chunk_t *chunk, uint8_t data) {
    if (chunk->size >= chunk->capacity) {
        chunk->capacity = NEW_CAPACITY(chunk->capacity);
        chunk->bytecode = realloc(chunk->bytecode, chunk->capacity * sizeof(*chunk->bytecode));
    }

    chunk->bytecode[chunk->size++] = data;
}

void free_chunk(chunk_t *chunk) {
    free(chunk->bytecode);
    free_constant_pool(&chunk->pool);
    init_chunk(chunk);
}

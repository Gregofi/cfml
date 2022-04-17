#include <stdbool.h>
#include <stdlib.h>

void heap_init(void* mem_pool, size_t mem_size);
void *heap_alloc(size_t size);
bool heap_free(void *blk);
size_t heap_done();
void* heap_realloc(void* blk, size_t new_size);
void* heap_calloc(size_t cnt, size_t size);

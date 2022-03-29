#include <stdbool.h>
#include <stdlib.h>

void heap_init(void* mem_pool, int mem_size);
void *heap_alloc(int size);
bool heap_free(void *blk);
int heap_done();
void* heap_realloc(void* blk, size_t new_size);
void* heap_calloc(size_t cnt, size_t size);

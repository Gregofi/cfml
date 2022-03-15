#include <stdbool.h>


void heap_init(void* mem_pool, int mem_size);
void *heap_alloc(int size);
bool heap_free(void *blk);
int heap_done();

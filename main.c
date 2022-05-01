#include <stdio.h>
#include "include/serializer.h"
#include "include/vm.h"
#include "include/dissasembler.h"
#include "include/buddy_alloc.h"

#define MEGABYTES(val) ((val) * 1024UL * 1024UL)
#define MEMORY MEGABYTES(1)

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s command file\n", argv[0]);
        exit(2);
    }

    /* Initialize memory */
#ifndef __SYSTEM_MEMORY__
    void* mempool = malloc(MEMORY);
    if (mempool == NULL) {
        fprintf(stderr, "Failed to allocate memory from the OS.\n");
    }
    heap_init(mempool, MEMORY);
#endif

    vm_t vm;
    init_vm(&vm);
    parse(&vm, argv[2]);

#ifdef __DEBUG__
    puts("Constant pool: ");
    for (size_t i = 0; i < vm.bytecode.pool.len; ++ i) {
        printf("%d: ", i);
        dissasemble_value(stderr, vm.bytecode.pool.data[i]);
        puts("");
    }

    dissasemble_global_variables(stderr, &vm);

    dissasemble_chunk(&vm.bytecode, "main chunk");
    fflush(stderr);
    puts("After parsing.\n");
#endif

    interpret_result_t result = interpret(&vm);
    if (result == INTERPRET_RUNTIME_ERROR) {
        fprintf(stderr, "Fatal: Runtime error occured.\n");
        exit(22);
    }

    free_vm(&vm);

#ifdef __DEBUG__
    fprintf(stderr, "%d blocks we're not freed.\n", heap_done());
#endif
    return 0;
}

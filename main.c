#include <stdio.h>
#include "include/serializer.h"
#include "include/vm.h"
#include "include/dissasembler.h"
#include "include/buddy_alloc.h"

#define MEGABYTES(val) ((val) * 1024UL * 1024UL)

const char *usage =
"usage: fml command file [options...]\n"
"    command:\n"
"        - execute - executes given bytecode\n"
"    options:\n"
"        --heap-log file - Logs heap activity into given file\n"
"        --heap-size size - Limits the heap with given size in megabytes\n";

void print_usage() {
    fprintf(stderr, "%s", usage);
}

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        print_usage();
        exit(2);
    }

    const char* log = NULL;
    size_t heap_size = MEGABYTES(2500);

    // Parse command line args
    for (ssize_t i = 1; i < argc; ++ i) {
        if (strcmp(argv[i], "--heap-log") == 0) {
            if (i + 1 >= argc) {
                print_usage();
                exit(2);
            }
            log = argv[++i];
        }
        if (strcmp(argv[i], "--heap-size") == 0) {
            if (i + 1 >= argc) {
                print_usage();
                exit(2);
            }
            heap_size = MEGABYTES(atol(argv[++i]));
            if (heap_size == 0) {
                print_usage();
                exit(2);
            }
        }
    }

    /* Initialize memory */
#ifndef __SYSTEM_MEMORY__
    void* mempool = malloc(heap_size);
    if (mempool == NULL) {
        fprintf(stderr, "Failed to allocate memory from the OS.\n");
    }
    heap_init(mempool, heap_size, log);
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

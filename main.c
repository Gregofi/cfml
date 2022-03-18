#include <stdio.h>
#include "include/serializer.h"
#include "include/vm.h"
#include "include/dissasembler.h"

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s command file\n", argv[0]);
        exit(2);
    }
    vm_t vm;
    init_vm(&vm);
    parse(&vm, argv[2]);

#ifdef __DEBUG__
    dissasemble_chunk(&vm.bytecode, "main chunk");
    fflush(stdout);
    puts("After parsing.\n");
#endif

    interpret_result_t result = interpret(&vm);
    if (result == INTERPRET_RUNTIME_ERROR) {
        fprintf(stderr, "Fatal: Runtime error occured.\n");
        exit(22);
    }
    free_vm(&vm);
    return 0;
}

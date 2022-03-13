#include <stdio.h>
#include "include/serializer.h"
#include "include/vm.h"
#include "include/dissasembler.h"

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        exit(2);
    }
    vm_t vm;
    init_vm(&vm);
    parse(&vm, argv[1]);

    dissasemble_chunk(&vm.bytecode, "main chunk");
    fflush(stdout);

    interpret_result_t result = interpret(&vm);
    if (result == INTERPRET_RUNTIME_ERROR) {
        fprintf(stderr, "Fatal: Runtime error occured.\n");
        exit(22);
    }
    free_vm(&vm);
    return 0;
}

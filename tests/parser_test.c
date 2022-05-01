#include "asserts.h"
#include <stdint.h>
#include <stdlib.h>
#include "include/vm.h"
#include "include/serializer.h"

TEST(basicTest) {
    uint8_t arr[] = {
        3, 0, // Const pool length
        0x02, 13, 0, 0, 0, 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\n',
        0x02, 3, 0, 0, 0, 'F', 'o', 'o', // Main function name
        0x03,   1, 0,   0,   0, 0,   1, 0, 0, 0,   0x02, 0, 0, // Function with one operation - printing hello world string.
        0, // Globals
        0, // Entry point
    };
    vm_t vm;
    init_vm(&vm);
    chunk_t chunk;
    init_chunk(&chunk);
    parse_constant_pool(&vm, arr);

    ASSERT_W(chunk.pool.len == 3);
    ASSERT_W(IS_STRING(chunk.pool.data[0]));
    ASSERT_W(strcmp(AS_CSTRING(chunk.pool.data[0]), "Hello world!\n") == 0);
    ASSERT_W(IS_STRING(chunk.pool.data[1]));
    ASSERT_W(strcmp(AS_CSTRING(chunk.pool.data[1]), "Foo") == 0);
    ASSERT_W(IS_FUNCTION(chunk.pool.data[2]));

    free_vm(&vm);
    free_chunk(&chunk);
    return EXIT_SUCCESS;
}


int main(void) {
    RUN_TEST(basicTest);
}

#include "asserts.h"
#include "include/bytecode.h"
#include "include/vm.h"
#include "include/constant.h"

TEST(DataStructureTest) {
    vm_t vm;
    init_vm(&vm);
    add_constant(&vm.bytecode.pool, INTEGER_VAL(2));
    add_constant(&vm.bytecode.pool, INTEGER_VAL(4));

    write_chunk(&vm.bytecode, OP_LITERAL);
    write_chunk(&vm.bytecode, 0);
    write_chunk(&vm.bytecode, 0);

    write_chunk(&vm.bytecode, OP_LITERAL);
    write_chunk(&vm.bytecode, 0);
    write_chunk(&vm.bytecode, 1);

    write_chunk(&vm.bytecode, OP_LITERAL);
    write_chunk(&vm.bytecode, 0);
    write_chunk(&vm.bytecode, 1);

    write_chunk(&vm.bytecode, OP_LITERAL);
    write_chunk(&vm.bytecode, 0);
    write_chunk(&vm.bytecode, 0);

    push(&vm, vm.bytecode.pool.data[0]);
    push(&vm, vm.bytecode.pool.data[1]);
    push(&vm, vm.bytecode.pool.data[0]);
    push(&vm, vm.bytecode.pool.data[1]);
    push(&vm, vm.bytecode.pool.data[0]);
    push(&vm, vm.bytecode.pool.data[1]);
    push(&vm, vm.bytecode.pool.data[0]);
    push(&vm, vm.bytecode.pool.data[1]);
    push(&vm, vm.bytecode.pool.data[0]);
    // Force reallocation
    value_t x1 = pop(&vm.op_stack);
    value_t x2 = pop(&vm.op_stack);
    ASSERT_W(IS_NUMBER(x1));
    ASSERT_W(AS_NUMBER(x1) == 2);
    ASSERT_W(IS_NUMBER(x2));
    ASSERT_W(AS_NUMBER(x2) == 4);
    free_vm(&vm);
    return EXIT_SUCCESS;
}


int main(void) {
    RUN_TEST(DataStructureTest);
}

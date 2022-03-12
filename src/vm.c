#include "include/vm.h"
#include "include/bytecode.h"
#include "include/memory.h"

void init_stack(op_stack_t* stack)
{
    memset(stack, 0, sizeof(*stack));
}

void free_stack(op_stack_t* stack)
{
    free(stack->data);
}

void push(op_stack_t* stack, constant_t c)
{
    if (stack->size >= stack->capacity) {
        stack->capacity = NEW_CAPACITY(stack->capacity);
        stack->data = realloc(stack->data, stack->capacity);
    }

    stack->data[stack->size++] = c;
}

constant_t pop(op_stack_t* stack)
{
    return stack->data[--stack->size];
}

void init_vm(vm_t* vm)
{
    memset(vm, 0, sizeof(*vm));
}

void free_vm(vm_t* vm)
{
    free_chunk(vm->bytecode);
}

interpret_result_t interpret(chunk_t* chunk)
{

}

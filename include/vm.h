#pragma once

#include "include/bytecode.h"

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR,
} interpret_result_t;

typedef struct {
    constant_t* data;
    size_t size;
    size_t capacity;
} op_stack_t;

typedef struct {
    chunk_t bytecode;
    // There can't be any writes to bytecode after this address is set.
    uint8_t* ip;
    op_stack_t op_stack;
} vm_t;

void init_stack(op_stack_t* stack);
void free_stack(op_stack_t* stack);
void push(op_stack_t* stack, constant_t c);
constant_t pop();

void init_vm(vm_t* vm);
void free_vm(vm_t* vm);
interpret_result_t interpret(vm_t* vm);

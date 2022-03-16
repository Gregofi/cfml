#pragma once

#include "include/bytecode.h"
#include "include/hashmap.h"

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR,
} interpret_result_t;

typedef struct {
    // Dynamic array of hash maps
    hash_map_t* frames;
    size_t capacity;
    size_t length;
} call_frames_t;

void push_frame(call_frames_t* call_frames);
void pop_frame(call_frames_t* call_frames);
bool find_var(obj_string_t* name, call_frames_t* call_frames, value_t* value);
bool add_var(obj_string_t* name, call_frames_t* call_frames, value_t* value);

typedef struct {
    value_t* data;
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
void push(op_stack_t* stack, value_t c);
value_t pop();

void init_vm(vm_t* vm);
void free_vm(vm_t* vm);
interpret_result_t interpret(vm_t* vm);

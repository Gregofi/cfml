#pragma once

#include "include/bytecode.h"
#include "include/hashmap.h"

#define MAX_FUN_ARGS 256
#define MAX_LOCALS 256
#define FRAMES_LIMIT 1024

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR,
} interpret_result_t;

typedef struct {
    value_t locals_vector[MAX_LOCALS];
    uint8_t* ip_backup;
} call_frame_t;

typedef struct {
    call_frame_t* frames;
    size_t capacity;
    size_t length;
} call_frames_t;

void push_frame(call_frames_t* call_frames, uint8_t* ip);
uint8_t* pop_frame(call_frames_t* call_frames);
void init_frames(call_frames_t* call_frames);
void free_frames(call_frames_t* call_frames);

typedef struct {
    value_t* data;
    size_t size;
    size_t capacity;
} op_stack_t;

void init_stack(op_stack_t* stack);
void free_stack(op_stack_t* stack);
void push(op_stack_t* stack, value_t c);
value_t pop();

typedef struct vm {
    chunk_t bytecode;
    // There can't be any writes to bytecode after this address is set.
    uint8_t* ip;
    op_stack_t op_stack;
    call_frames_t frames;
    // Contains name of the global variables as key and it's values.
    hash_map_t global_var;
    // List of all fml objects
    obj_t* objects;

    // ==== GC Internals ====
    // The GC worklist
    size_t gray_cnt;
    size_t gray_capacity;
    obj_t** gray_stack;

} vm_t;

void init_vm(vm_t* vm);
void free_vm(vm_t* vm);
interpret_result_t interpret(vm_t* vm);

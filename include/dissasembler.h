#pragma once

#include <stdio.h>
#include <strings.h>

#include "include/bytecode.h"
#include "include/memory.h"
#include "include/vm.h"

void dissasemble_global_variables(FILE* stream, vm_t* vm);
void dissasemble_chunk(chunk_t *chunk, const char* name);
size_t dissasemble_instruction(chunk_t* chunk, size_t offset);
void dissasemble_stack(op_stack_t* op_stack);
void dissasemble_frames(call_frames_t* frames);
void dissasemble_object(FILE* stream, obj_t* obj);
void dissasemble_value(FILE* stream, value_t value);

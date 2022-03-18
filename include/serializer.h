#pragma once

#include "include/constant.h"
#include "include/bytecode.h"
#include "include/vm.h"

#include <stddef.h>
#include <stdio.h>

typedef enum {
    CD_INTEGER = 0x00,
    CD_NULL = 0x01,
    CD_STRING = 0x02,
    CD_METHOD = 0x03,
    CD_SLOT = 0x04,
    CD_CLASS = 0x05,
    CD_BOOLEAN = 0x06,
} constant_serialization_type_t;

void parse(vm_t* vm, const char* filename);
uint8_t* parse_globals(vm_t *vm, chunk_t* chunk, uint8_t* code);
uint8_t* parse_constant_pool(vm_t* vm, uint8_t *file, chunk_t *chunk);
/// Parses 'intruction_count' instruciton from 'bytecode'.
/// @return Returns pair containing number of bytes read and size
///         of the new instructions stored (jumps are smaller when not parsed)
size_t_pair_t parse_bytecode(uint8_t* bytecode, size_t instruction_count, chunk_t* chunk, hash_map_t* labels);

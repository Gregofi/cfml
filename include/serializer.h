#pragma once

#include <bits/types/FILE.h>
#include <stddef.h>
#include <stdio.h>
#include "constant.h"
#include "bytecode.h"

typedef enum {
    CD_INTEGER = 0x00,
    CD_NULL = 0x01,
    CD_STRING = 0x02,
    CD_METHOD = 0x03,
    CD_SLOT = 0x04,
    CD_CLASS = 0x05,
    CD_BOOLEAN = 0x06,
} constant_serialization_type_t;

/// Reads bytes from file and returns a pointer to the bytes.
uint8_t* read_file(const char* name);
/// Returns number of bytes read.
size_t parse_bytecode(uint8_t* bytecode, size_t instruction_count);
static void parse_constant_pool(uint8_t *file, chunk_t *chunk);

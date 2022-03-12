#pragma once
#include <stdio.h>

#include "include/bytecode.h"

void dissasemble_chunk(chunk_t *chunk, const char* name) {

}

static size_t simple_instruction(const char *name, size_t offset) {
    printf("%s\n", name);
    return offset + 1;
}

size_t dissasemble_instruction(chunk_t* chunk, size_t offset) {
    printf("%04lX ", offset);

    uint8_t opcode = chunk->bytecode[offset];
    switch (opcode) {
            case OP_RETURN:
                return simple_instruction("OP_RETURN", offset);
            case OP_ARRAY:
                return simple_instruction("OP_ARRAY", offset);
            case OP_DROP:
                return simple_instruction("OP_DROP", offset);
            case OP_LITERAL:
            case OP_GET_LOCAL:
            case OP_SET_LOCAL:
            case OP_GET_GLOBAL:
            case OP_SET_GLOBAL:
            case OP_LABEL:
            case OP_JUMP:
            case OP_BRANCH:
            case OP_OBJECT:
            case OP_GET_FIELD:
            case OP_SET_FIELD:
            case OP_CALL_FUNCTION:
            case OP_PRINT:
            case OP_CALL_METHOD:
            default:

    }
}
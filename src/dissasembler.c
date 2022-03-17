#include <stdio.h>
#include <strings.h>

#include "include/bytecode.h"
#include "include/memory.h"

static size_t simple_instruction(const char *name, size_t offset) {
    printf("%s", name);
    return offset + 1;
}

/// For three bytes instruction having as operand index to constant pool.
static size_t index_instruction(const char* name, uint8_t* bytecode, size_t offset) {
    printf("%s %04d", name, READ_2BYTES(bytecode + offset + 1));
    return offset + 3;
}

size_t dissasemble_instruction(chunk_t* chunk, size_t offset) {
    printf("%04ld ", offset);

    uint8_t opcode = chunk->bytecode[offset];
    switch (opcode) {
            case OP_RETURN:
                return simple_instruction("OP_RETURN", offset);
            case OP_ARRAY:
                return simple_instruction("OP_ARRAY", offset);
            case OP_DROP:
                return simple_instruction("OP_DROP", offset);
            case OP_LITERAL:
                return index_instruction("OP_LITERAL", chunk->bytecode, offset);
            case OP_GET_LOCAL:
                return index_instruction("OP_GET_LOCAL", chunk->bytecode, offset);
            case OP_SET_LOCAL:
                return index_instruction("OP_SET_LOCAL", chunk->bytecode, offset);
            case OP_GET_GLOBAL:
                return index_instruction("OP_GET_GLOBAL", chunk->bytecode, offset);
            case OP_SET_GLOBAL:
                return index_instruction("OP_SET_GLOBAL", chunk->bytecode, offset);
            case OP_LABEL: {
                uint16_t index = READ_2BYTES(chunk->bytecode + offset + 1);
                const char* label_name = AS_CSTRING(chunk->pool.data[index]);
                printf("OP_LABEL: %s", label_name);
                return offset + 3;
            }
            case OP_OBJECT:
                return index_instruction("OP_OBJECT", chunk->bytecode, offset);
            case OP_GET_FIELD:
                return index_instruction("OP_GET_FIELD", chunk->bytecode, offset);
            case OP_SET_FIELD:
                return index_instruction("OP_SET_FIELD", chunk->bytecode, offset);
            case OP_JUMP: {
                printf("OP_JUMP ");
                uint32_t index = chunk->bytecode[offset + 1] << 16 
                               | chunk->bytecode[offset + 2] << 8 
                               | chunk->bytecode[offset + 3];
                printf("%04d", index);
                return offset + 4;
            }
            case OP_BRANCH: {
                printf("OP_BRANCH ");
                uint32_t index = chunk->bytecode[offset + 1] << 16 
                               | chunk->bytecode[offset + 2] << 8 
                               | chunk->bytecode[offset + 3];
                printf("%04d", index);
                return offset + 4;
            }
            case OP_CALL_FUNCTION:
                printf("OP_CALL_FUNCTION");
                return offset + 4;
            case OP_PRINT:
                printf("OP_PRINT");
                return offset + 4;
            case OP_CALL_METHOD:
                printf("OP_CALL_METHOD");
                return offset + 4;
            default:
                fprintf(stderr, "Unknown instruction with code '0x%X' to dissasemble.\n", opcode);
                exit(53);
    }
}

void dissasemble_chunk(chunk_t *chunk, const char* name) {
    printf("=== %s ===\n", name);
    for(size_t idx = 0; idx < chunk->size;) {
        idx = dissasemble_instruction(chunk, idx);
        puts("");
    }
}

#include <stdio.h>

#include "include/serializer.h"
#include "include/bytecode.h"
#include "include/constant.h"
#include "include/memory.h"
#include "include/vm.h"

static uint8_t* read_file(const char* name) {
    size_t size = 0, bytes_read = 0;
    uint8_t* buffer = NULL;
    FILE *f = fopen(name, "rb");

    if (!f) {
        fprintf(stderr, "Couldn't open file '%s'.\n", name);
        goto FILE_ERROR; 
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    rewind(f);

    buffer = (uint8_t*)malloc(size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read file.\n");
        goto MALLOC_ERROR;
    }

    bytes_read = fread(buffer, sizeof(*buffer), size, f);
    buffer[bytes_read] = '\0';
    if (bytes_read != size) {
        fprintf(stderr, "Could not read file '%s'.\n", name);
        goto READ_ERROR;
    }
    fclose(f);
    return buffer;

READ_ERROR:
    free(buffer);
MALLOC_ERROR:
    fclose(f);
FILE_ERROR:
    exit(33);
}

static size_t parse_bytecode(uint8_t* bytecode, size_t instruction_count, chunk_t* chunk) {
    size_t byte_size = 0;
    while (instruction_count != 0) {
        switch (bytecode[byte_size]) {
            // one byte instructions
            case OP_RETURN:
            case OP_ARRAY:
            case OP_DROP:
                write_chunk(chunk, bytecode[byte_size]);
                byte_size += 1;
                break;
            // three byte instructions
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
                write_chunk(chunk, bytecode[byte_size]);
                // Indexes are in little endian.
                write_chunk(chunk, bytecode[byte_size + 1]);
                write_chunk(chunk, bytecode[byte_size + 2]);
                byte_size += 3;
                break;
            // four byte instruction
            case OP_CALL_FUNCTION:
            case OP_PRINT:
            case OP_CALL_METHOD:
                write_chunk(chunk, bytecode[byte_size]);
                write_chunk(chunk, bytecode[byte_size + 1]);
                write_chunk(chunk, bytecode[byte_size + 2]);
                write_chunk(chunk, bytecode[byte_size + 3]);
                byte_size += 4;
                break;
            default:
                fprintf(stderr, "Unknown instruction '0x%X' to serialize.\n", bytecode[byte_size]);
        }
        instruction_count -= 1;
    }
    return byte_size;
}

static uint8_t* parse_constant_pool(uint8_t *file, chunk_t *chunk) {
    // Read size of constant pool
    uint16_t size = READ_2BYTES(file);
    file += 2;

    while (size != 0) {
        switch (*file) {
            case CD_BOOLEAN:
                add_constant(&chunk->pool, BOOL_VAL(*(file + 1)));
                file += 2;
                break;
            case CD_INTEGER:
                add_constant(&chunk->pool, INTEGER_VAL(READ_4BYTES(file + 1)));
                file += 5;
                break;
            case CD_NULL:
                add_constant(&chunk->pool, NULL_VAL);
                file += 1;
                break;
            case CD_STRING: {
                size_t length = READ_4BYTES(file + 1);
                const char* str = (char*)(file + 5);
                add_constant(&chunk->pool, OBJ_STRING_VAL(length, str));
                file += 5 + length;
                break;
            }
            case CD_METHOD: {
                constant_t fun = OBJ_FUN_VAL();
                obj_function_t *fun_obj = AS_FUNCTION(fun);
                fun_obj->name = READ_2BYTES(file + 1);
                fun_obj->args = READ_BYTE(file + 3);
                fun_obj->locals = READ_2BYTES(file + 4);
                fun_obj->entry_point = chunk->size; 

                uint32_t bytecode_length = READ_4BYTES(file + 6);
                // Read the bytecode and update the length to bytes instead of instruction count
                bytecode_length = parse_bytecode(file + 10, bytecode_length, chunk);
                fun_obj->length = bytecode_length;
                file += 10 + bytecode_length;
                add_constant(&chunk->pool, fun);

                break;
            }
            case CD_CLASS:
            case CD_SLOT:
            default:
                fprintf(stderr, "Unknown tag 0x%X for constant object.", *file);
                exit(2);
        }
        size -= 1;
    }
    return file;
}

void parse(vm_t* vm, const char* name) {
    chunk_t chunk;
    init_chunk(&chunk);
    uint8_t *file = read_file(name);
    uint8_t *file_ptr = file;
    file_ptr = parse_constant_pool(file_ptr, &chunk);
    // For now just skip globals
    file_ptr += READ_2BYTES(file_ptr) + 2;
    uint16_t entry_point = READ_2BYTES(file_ptr);
    vm->bytecode = chunk;
    vm->ip = &chunk.bytecode[AS_FUNCTION((chunk.pool.data[entry_point]))->entry_point];
    free(file);
}

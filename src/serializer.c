#include <stdio.h>
#include <stdint.h>

#include "include/hashmap.h"
#include "include/serializer.h"
#include "include/bytecode.h"
#include "include/constant.h"
#include "include/memory.h"
#include "include/vm.h"
#include "include/dissasembler.h"

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

/// Single pass through the entire bytecode to prepare jumping instructions.
static void prepare_jumps(const chunk_t* chunk, hash_map_t* jump_map) {
    for (size_t i = 0; i < chunk->size;) {
        switch (chunk->bytecode[i]) {
            // one byte instructions
            case OP_RETURN:
            case OP_ARRAY:
            case OP_DROP:
                i += 1;
                break;
            // three byte instructions
            case OP_LITERAL:
            case OP_GET_LOCAL:
            case OP_SET_LOCAL:
            case OP_GET_GLOBAL:
            case OP_SET_GLOBAL:
            case OP_LABEL: {
                i += 3;
                break;
            }
            case OP_OBJECT:
            case OP_GET_FIELD:
            case OP_SET_FIELD:
                i += 3;
                break;
            // four byte instruction
            case OP_CALL_FUNCTION:
            case OP_PRINT:
            case OP_CALL_METHOD:
                i += 4;
                break;
            case OP_JUMP:
            case OP_BRANCH: {
                uint16_t index = READ_2BYTES(chunk->bytecode + i + 1);
                obj_string_t* label = AS_STRING(chunk->pool.data[index]);
                value_t row;
                if (!hash_map_fetch(jump_map, label, &row)) {
                    fprintf(stderr, "Couldn't find jump label '%s' in hashmap.\n", label->data);
                    exit(54);
                }
                
                chunk->bytecode[i + 1] = (uint8_t)(row.num >> 16);
                chunk->bytecode[i + 2] = (uint8_t)(row.num >> 8);
                chunk->bytecode[i + 3] = (uint8_t)row.num;
                i += 4;
                break;
            }
            default:
                fprintf(stderr, "Unknown instruction '0x%X' to serialize in jumps prepare.\n", chunk->bytecode[i]);
                break;
        }
    }
}

size_t_pair_t parse_bytecode(uint8_t* bytecode, size_t instruction_count, chunk_t* chunk, hash_map_t* labels) {
    size_t byte_size = 0;
    size_t jumps_cnt = 0;
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
            // Special cases
            case OP_LABEL: {
                obj_string_t* str = AS_STRING(chunk->pool.data[READ_2BYTES(bytecode + byte_size + 1)]);
                hash_map_insert(labels, str, INTEGER_VAL(chunk->size));
                write_chunk(chunk, bytecode[byte_size]);
                write_chunk(chunk, bytecode[byte_size + 1]);
                write_chunk(chunk, bytecode[byte_size + 2]);
                byte_size += 3;
                break;
            }
            // Jump instructions will receive their destination in another pass
            // Now they are also 4 bytes. 
            case OP_JUMP:
            case OP_BRANCH:
                write_chunk(chunk, bytecode[byte_size]);
                write_chunk(chunk, bytecode[byte_size + 1]);
                write_chunk(chunk, bytecode[byte_size + 2]);
                write_chunk(chunk, 0xFF); // Dummy value
                byte_size += 3;
                jumps_cnt += 1;
                break;

            default:
                fprintf(stderr, "Unknown instruction '0x%X' to serialize.\n", bytecode[byte_size]);
                break;
        }
        instruction_count -= 1;
    }
    return (size_t_pair_t){byte_size, byte_size + jumps_cnt};
}

uint8_t* parse_constant_pool(vm_t* vm, uint8_t *file, chunk_t *chunk) {
    // Read size of constant pool
    uint16_t size = READ_2BYTES(file);
    file += 2;
    hash_map_t labels;
    init_hash_map(&labels);

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
                add_constant(&chunk->pool, OBJ_STRING_VAL(length, str, hash_string(str)));
                file += 5 + length;
                break;
            }
            case CD_METHOD: {
                value_t fun = OBJ_FUN_VAL();
                obj_function_t *fun_obj = AS_FUNCTION(fun);
                fun_obj->name = READ_2BYTES(file + 1);
                fun_obj->args = READ_BYTE(file + 3);
                fun_obj->locals = READ_2BYTES(file + 4);
                fun_obj->entry_point = chunk->size; 

                uint32_t bytecode_length = READ_4BYTES(file + 6);
                // Read the bytecode and update the length to bytes instead of instruction count
                size_t_pair_t p = parse_bytecode(file + 10, bytecode_length, chunk, &labels);
                fun_obj->length = p.second;
                file += 10 + p.first;
                add_constant(&chunk->pool, fun);
                // Add function to global map
                hash_map_insert(&vm->global_var, AS_STRING(chunk->pool.data[fun_obj->name]), fun);
                break;
            }
            case CD_CLASS:
                NOT_IMPLEMENTED();
            case CD_SLOT: {
                value_t slot = OBJ_SLOT_VAL(READ_2BYTES(file + 1));
                add_constant(&chunk->pool, slot);
                // Either global or field, add it to globals
                hash_map_insert(&vm->global_var, AS_STRING(chunk->pool.data[AS_SLOT(slot)->index]), NULL_VAL);
                file += 3;
                break;
            }
            default:
                fprintf(stderr, "Unknown tag 0x%X for constant object.\n", *file);
                exit(2);
        }
        size -= 1;
    }

    // Change labels to indexes
    prepare_jumps(chunk, &labels);

    free_hash_map(&labels);
    return file;
}

uint8_t* parse_globals(vm_t *vm, chunk_t* chunk, uint8_t* code) {
    uint16_t globals = READ_2BYTES(code);
    code += 2;
    for(uint16_t i = 0; i < globals; ++ i) {
        uint16_t index = READ_2BYTES(code);
        write_global(&chunk->globals, index);
        code += 2;
    }
    return code;
}

void parse(vm_t* vm, const char* name) {
    chunk_t chunk;
    init_chunk(&chunk);
    uint8_t *file = read_file(name);
    uint8_t *file_ptr = file;
    file_ptr = parse_constant_pool(vm, file_ptr, &chunk);

    file_ptr = parse_globals(vm, &chunk, file_ptr);
    // Read entry point
    uint16_t entry_point = READ_2BYTES(file_ptr);
    vm->bytecode = chunk;

    vm->ip = &chunk.bytecode[AS_FUNCTION((chunk.pool.data[entry_point]))->entry_point];
    free(file);
}

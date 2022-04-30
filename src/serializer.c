#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "include/hashmap.h"
#include "include/serializer.h"
#include "include/bytecode.h"
#include "include/constant.h"
#include "include/memory.h"
#include "include/vm.h"
#include "include/dissasembler.h"
#include "include/objects.h"
#include "include/buddy_alloc.h"

/**
 * Pending global variables to be saved to global_var hashmap
 * When encountering a slot or function, we do not know if it
 * belong to a class or is global. We save it here and then sort
 * it out at the end of parsing.
 */
typedef struct globals_pending {
    size_t size;
    size_t capacity;
    int* data;
} globals_pending_t;

static void init_pending(globals_pending_t* globals) {
    memset(globals, 0, sizeof(*globals));
}

static void push_pending(globals_pending_t* globals, int value) {
    if (globals->capacity <= globals->size) {
        globals->capacity = NEW_CAPACITY(globals->capacity);
        globals->data = heap_realloc(globals->data, globals->capacity * sizeof(*globals->data));
    }
    globals->data[globals->size++] = value;
}

static void remove_pending(globals_pending_t* globals, int value) {
    for (size_t i = 0; i < globals->size; ++i) {
        if (globals->data[i] == value) {
            for (size_t j = i; i < globals->size - 1; ++ i) {
                globals->data[j] = globals->data[j + 1];
            }
            globals->size -= 1;
            return;
        }
    }
}

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
            case OP_LABEL:
                i += 3;
                break;
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

int compare_strings(const void *x, const void *y) {
    const obj_string_t* str_x = *(obj_string_t**)x;
    const obj_string_t* str_y = *(obj_string_t**)y;

    return strcmp(str_x->data, str_y->data);
}

uint8_t* parse_constant_pool(vm_t* vm, uint8_t *file, chunk_t *chunk) {
    // Read size of constant pool
    uint16_t size = READ_2BYTES(file);
    file += 2;
    hash_map_t labels;
    init_hash_map(&labels);

    globals_pending_t pending;
    init_pending(&pending);

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
                add_constant(&chunk->pool, OBJ_STRING_VAL(length, str, hash_string(str), vm));
                file += 5 + length;
                break;
            }
            case CD_METHOD: {
                value_t fun = OBJ_FUN_VAL(vm);
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
                size_t ci = add_constant(&chunk->pool, fun);
                // Add function to globals pending
                push_pending(&pending, ci);
                break;
            }
            case CD_CLASS: {
                uint16_t members_cnt = READ_2BYTES(file + 1);
                value_t class = OBJ_CLASS_VAL(vm);
                obj_class_t* as_class = AS_CLASS(class);
                // Save methods and member variables
                file += 3;
                for (size_t i = 0; i < members_cnt; ++ i) {
                    uint16_t index = READ_2BYTES(file);
                    file += 2;
                    if (IS_FUNCTION(chunk->pool.data[index])) {
                        obj_function_t* fun = AS_FUNCTION(chunk->pool.data[index]);
                        hash_map_insert(&as_class->methods, AS_STRING(chunk->pool.data[fun->name]), chunk->pool.data[index]);
                    } else if (IS_SLOT(chunk->pool.data[index])) {
                        obj_slot_t* slot = AS_SLOT(chunk->pool.data[index]);
                        obj_string_t* name = AS_STRING(chunk->pool.data[slot->index]);
                        as_class->fields[as_class->size++] = name;
                    } else {
                        fprintf(stderr, "Wrong object type on function.\n");
                        exit(8);
                    }
                    // This will certainly not be global object, so remove it from global pendig
                    remove_pending(&pending, index);
                }
                add_constant(&chunk->pool, class);
                break;
            }
            case CD_SLOT: {
                value_t slot = OBJ_SLOT_VAL(READ_2BYTES(file + 1), vm);
                size_t ci = add_constant(&chunk->pool, slot);
                // Either global or field, add it to globals
                push_pending(&pending, ci);
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

    // Insert globals into hashmap
    for (size_t i = 0; i < pending.size; ++i) {
        value_t val = chunk->pool.data[pending.data[i]];
        if (IS_SLOT(val)) {
            hash_map_insert(&vm->global_var, AS_STRING(chunk->pool.data[AS_SLOT(val)->index]), NULL_VAL);
        } else if (IS_FUNCTION(val)) {
            hash_map_insert(&vm->global_var, AS_STRING(chunk->pool.data[AS_FUNCTION(val)->name]), val);
        } else {
            fprintf(stderr, "Unknown object in globals pending.\n");
            exit(22);
        }
    }

    free_hash_map(&labels);
    return file;
}

uint8_t* parse_globals(chunk_t* chunk, uint8_t* code) {
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

    file_ptr = parse_globals(&chunk, file_ptr);
    // Read entry point
    uint16_t entry_point = READ_2BYTES(file_ptr);
    vm->bytecode = chunk;

    vm->ip = &chunk.bytecode[AS_FUNCTION((chunk.pool.data[entry_point]))->entry_point];
    free(file);
}

#include <stdio.h>

#include "include/constant.h"
#include "include/hashmap.h"
#include "include/vm.h"
#include "include/bytecode.h"
#include "include/memory.h"
#include "include/dissasembler.h"

call_frame_t* get_top_frame(call_frames_t* call_frames) {
    return &call_frames->frames[call_frames->length - 1];
}

call_frame_t* get_global_frame(call_frames_t* call_frames) {
    return &call_frames->frames[0];
}

void push_frame(call_frames_t* call_frames, uint8_t* ip) {
    if (call_frames->length >= call_frames->capacity) {
        call_frames->capacity = NEW_CAPACITY(call_frames->capacity);
        call_frames->frames = realloc(call_frames->frames, sizeof(*call_frames->frames) * call_frames->capacity);
    }

    for(size_t i = 0; i < MAX_LOCALS; ++ i) {
        call_frames->frames[call_frames->length].locals_vector[i] = NULL_VAL;
    }

    call_frames->frames[call_frames->length].ip_backup = ip;
    call_frames->length += 1;
}

uint8_t* pop_frame(call_frames_t* call_frames) {
    return call_frames->frames[--call_frames->length].ip_backup;
}

void init_frames(call_frames_t* call_frames)
{
    memset(call_frames, 0, sizeof(*call_frames));
}

void free_frames(call_frames_t* call_frames)
{
    free(call_frames->frames);
    init_frames(call_frames);
}

void init_stack(op_stack_t* stack)
{
    stack->capacity = 0;
    stack->size = 0;
    stack->data = NULL;
}

void free_stack(op_stack_t* stack)
{
    free(stack->data);
}

void push(op_stack_t* stack, value_t c)
{
    if (stack->size >= stack->capacity) {
        stack->capacity = NEW_CAPACITY(stack->capacity);
        stack->data = realloc(stack->data, stack->capacity * sizeof(*stack->data));
    }

    stack->data[stack->size++] = c;
}

value_t pop(op_stack_t* stack)
{
    if (stack->size == 0) {
        fprintf(stderr, "Popping from empty stack.\n");
        exit(77);
    }
    return stack->data[--stack->size];
}

value_t peek(op_stack_t* stack) {
    return stack->data[stack->size - 1];
}

void init_vm(vm_t* vm)
{
    vm->ip = NULL;
    init_stack(&vm->op_stack);
    init_chunk(&vm->bytecode);
    init_frames(&vm->frames);
    init_hash_map(&vm->global_var);
}

void free_vm(vm_t* vm)
{
    free_stack(&vm->op_stack);
    free_chunk(&vm->bytecode);
    free_hash_map(&vm->global_var);
    free_frames(&vm->frames);
    init_vm(vm);
}

#define READ_BYTE_IP(vm) (*((vm)->ip++))
#define READ_WORD_IP(vm) ((vm)->ip += 2, (*((vm)->ip - 2) | (*((vm)->ip - 1) << 8)))

bool interpret_print(vm_t* vm) {
    value_t obj = vm->bytecode.pool.data[READ_WORD_IP(vm)];
    if (!IS_STRING(obj)) {
        fprintf(stderr, "Print keyword accepts only string as it's first argument.\n");
        return false;
    }
    const char* str = AS_CSTRING(obj);
    uint8_t arg_count = READ_BYTE_IP(vm);
    for(const char* ptr = str; *ptr != '\0'; ptr ++) {
        if (*ptr == '~') {
            value_t val = pop(&vm->op_stack);
            switch (val.type) {
                case TYPE_INTEGER:
                    printf("%d", AS_NUMBER(val));
                    break;
                case TYPE_NULL:
                    printf("null");
                    break;
                case TYPE_BOOLEAN:
                    printf(AS_BOOL(val) ? "true" : "false");
                    break;
                case TYPE_OBJECT:
                default:
                    fprintf(stderr, "Uknown value type to print.\n");
                    return false;
            }
            arg_count -= 1;
        } else if (*ptr == '\\') {
            ptr += 1;
            switch (*ptr) {
                case '\\':
                    putchar('\\');
                    break;
                case 'n':
                    putchar('\n');
                    break;
                case 'r':
                    putchar('\r');
                    break;
                case 't':
                    putchar('\t');
                    break;
                case '~':
                    putchar('~');
                    break;
                case '"':
                    putchar('"');
                    break;
                default:
                    fprintf(stderr, "Unknown escape sequence '\\%c'.\n", *ptr);
                    break;
            }
        } else {
            putchar(*ptr);
        }
    }

    if (arg_count != 0) {
        fprintf(stderr, "Wrong number of arguments to print statement.\n");
        return false;
    }

    push(&vm->op_stack, NULL_VAL);

    return true;
}

interpret_result_t interpret_function_call(vm_t* vm, obj_function_t *func, uint8_t arg_cnt) {
    push_frame(&vm->frames, vm->ip);
    // Populate the new frame with arguments
    call_frame_t* top_frame = get_top_frame(&vm->frames);
    for (int i = arg_cnt - 1; i >= 0; -- i) {
        top_frame->locals_vector[i] = pop(&vm->op_stack);
    }

    // Set instruction pointer to function entry point.
    vm->ip = &vm->bytecode.bytecode[func->entry_point];

    return INTERPRET_OK;
}

obj_function_t* get_function(obj_string_t* name, vm_t* vm) {
    for (uint16_t i = 0; i < vm->bytecode.globals.length; ++ i) {
        value_t obj = vm->bytecode.pool.data[vm->bytecode.globals.indexes[i]];
        if (IS_FUNCTION(obj)) {
            obj_function_t* fun = AS_FUNCTION(obj);
            const char* fun_name = AS_CSTRING(vm->bytecode.pool.data[fun->name]);
            if (strcmp(fun_name, name->data) == 0) {
                return fun;
            }
        }
    }

    fprintf(stderr, "Function with name %s doesn't exist.\n", name->data);
    exit(51);
}

interpret_result_t interpret(vm_t* vm)
{
    push_frame(&vm->frames, NULL);
    for (;(size_t)(vm->ip - vm->bytecode.bytecode) < vm->bytecode.size;) {
#ifdef __DEBUG__
        dissasemble_instruction(&vm->bytecode, vm->ip - vm->bytecode.bytecode);
        puts("");
        dissasemble_stack(&vm->op_stack);
#endif // __DEBUG__
        switch (READ_BYTE_IP(vm)) {
            case OP_RETURN: {
                uint8_t* old_ip = pop_frame(&vm->frames);
                // If global frame is popped.
                if (old_ip == NULL) {
                    return INTERPRET_OK;
                }
                vm->ip = old_ip;
                break;
            }
            case OP_LABEL:
                // We have updated jumps,
                READ_WORD_IP(vm);
                break;
            case OP_DROP:
                pop(&vm->op_stack);
                break;
            case OP_LITERAL: {
                uint16_t index = READ_WORD_IP(vm);
                push(&vm->op_stack, vm->bytecode.pool.data[index]);
                break;
            }
            case OP_PRINT:
                if (!interpret_print(vm)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            case OP_GET_LOCAL: {
                uint16_t index = READ_WORD_IP(vm);
                push(&vm->op_stack, get_top_frame(&vm->frames)->locals_vector[index]);
                break;
            }
            case OP_SET_LOCAL: {
                uint16_t index = READ_WORD_IP(vm);
                get_top_frame(&vm->frames)->locals_vector[index] = peek(&vm->op_stack);
                break;
            }
            case OP_GET_GLOBAL: {
                uint16_t index = READ_WORD_IP(vm);
                obj_string_t* name = AS_STRING(vm->bytecode.pool.data[index]);
                value_t val;
                hash_map_fetch(&vm->global_var, name, &val);
                push(&vm->op_stack, val);
                break;
            }
            case OP_SET_GLOBAL: {
                uint16_t index = READ_WORD_IP(vm);
                obj_string_t* name = AS_STRING(vm->bytecode.pool.data[index]);
                value_t val = peek(&vm->op_stack);
                hash_map_update(&vm->global_var, name, val);
                break;
            }
            case OP_BRANCH: {
                value_t val = pop(&vm->op_stack);
                if(IS_FALSY(val)) {
                    vm->ip += 3;
                    break;
                }
            // Else fall through
            }
            case OP_JUMP: {
                // Jump index is not in little endian.
                size_t index = *vm->ip << 16 | *(vm->ip + 1) << 8 | *(vm->ip + 2);
                vm->ip = &vm->bytecode.bytecode[index];
                break;
            }
            case OP_OBJECT:
                NOT_IMPLEMENTED();
            case OP_GET_FIELD:
                NOT_IMPLEMENTED();
            case OP_SET_FIELD:
                NOT_IMPLEMENTED();
            case OP_CALL_FUNCTION: {
                uint16_t index = READ_WORD_IP(vm);
                obj_string_t* fun_name = AS_STRING(vm->bytecode.pool.data[index]);
                uint8_t arg_cnt = READ_BYTE_IP(vm);
                // Fetch function from global pool
                obj_function_t* fun = get_function(fun_name, vm);
                interpret_function_call(vm, fun, arg_cnt);
                break;
            }

            case OP_ARRAY: {
                value_t init = pop(&vm->op_stack);
                value_t size = pop(&vm->op_stack);

                value_t array = OBJ_ARRAY_VAL(AS_NUMBER(size), init);
                push(&vm->op_stack, array);
                break;
            }
            case OP_CALL_METHOD: {
                NOT_IMPLEMENTED();
            }
            default:
                fprintf(stderr, "Unknown instruction to interpret.\n");
                return INTERPRET_RUNTIME_ERROR;

        }
    }
    return INTERPRET_OK;
}

#undef READ_BYTE_IP
#undef READ_WORD_IP

#include <stdio.h>

#include "include/constant.h"
#include "include/vm.h"
#include "include/bytecode.h"
#include "include/memory.h"

void init_stack(op_stack_t* stack)
{
    memset(stack, 0, sizeof(*stack));
}

void free_stack(op_stack_t* stack)
{
    free(stack->data);
}

void push(op_stack_t* stack, constant_t c)
{
    if (stack->size >= stack->capacity) {
        stack->capacity = NEW_CAPACITY(stack->capacity);
        stack->data = realloc(stack->data, stack->capacity);
    }

    stack->data[stack->size++] = c;
}

constant_t pop(op_stack_t* stack)
{
    return stack->data[--stack->size];
}

void init_vm(vm_t* vm)
{
    vm->ip = NULL;
    init_stack(&vm->op_stack);
    init_chunk(&vm->bytecode);
}

void free_vm(vm_t* vm)
{
    free_chunk(&vm->bytecode);
    free_stack(&vm->op_stack);
    init_vm(vm);
}

#define READ_BYTE_IP(vm) (*((vm)->ip++))
#define READ_WORD_IP(vm) ((vm)->ip += 2, (*((vm)->ip - 2) | (*((vm)->ip - 1) << 8)))

bool interpret_print(vm_t* vm) {
    constant_t obj = vm->bytecode.pool.data[READ_WORD_IP(vm)];
    if (!IS_STRING(obj)) {
        fprintf(stderr, "Print keyword accepts only string as it's first argument.");
        return false;
    }
    const char* str = AS_CSTRING(obj);
    uint8_t arg_count = READ_BYTE_IP(vm);
    printf("%s", str);
    for(const char* ptr = str; *ptr != '\0'; ptr ++) {
        if (*ptr == '~') {
            constant_t val = pop(&vm->op_stack);
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
        } else {
            putchar(*ptr);
        }
    }

    if (arg_count != 0) {
        fprintf(stderr, "Wrong number of arguments to print statement.\n");
        return false;
    }

    return true;
}

interpret_result_t interpret(vm_t vm)
{
    for(;(size_t)(vm.ip - vm.bytecode.bytecode) < vm.bytecode.size;) {
        switch(READ_BYTE_IP(&vm)) {
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_LABEL:
                break;
            case OP_DROP:
                pop(&vm.op_stack);
                break;
            case OP_LITERAL: {
                uint16_t index = READ_WORD_IP(&vm);
                push(&vm.op_stack, vm.bytecode.pool.data[index]);
            }
            case OP_GET_LOCAL:
            case OP_SET_LOCAL:
            case OP_GET_GLOBAL:
            case OP_SET_GLOBAL:
            case OP_JUMP:
            case OP_BRANCH:
            case OP_OBJECT:
            case OP_GET_FIELD:
            case OP_SET_FIELD:
            case OP_CALL_FUNCTION:
            case OP_PRINT:
                if (!interpret_print(&vm)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            case OP_ARRAY:
            case OP_CALL_METHOD:
            default:
                fprintf(stderr, "Unknown instruction to interpret.\n");
                return INTERPRET_RUNTIME_ERROR;

        }
    }
    return INTERPRET_OK;
}

#undef READ_BYTE_IP
#undef READ_WORD_IP

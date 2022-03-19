#include <stdio.h>

#include "include/constant.h"
#include "include/hashmap.h"
#include "include/vm.h"
#include "include/bytecode.h"
#include "include/memory.h"
#include "include/dissasembler.h"
#include "include/objects.h"

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

void init_vm(vm_t* vm) {
    vm->ip = NULL;
    init_stack(&vm->op_stack);
    init_chunk(&vm->bytecode);
    init_frames(&vm->frames);
    init_hash_map(&vm->global_var);
}

void free_vm(vm_t* vm) {
    free_stack(&vm->op_stack);
    free_chunk(&vm->bytecode);
    free_hash_map(&vm->global_var);
    free_frames(&vm->frames);
    init_vm(vm);
}

int compare_str_pointers(const void* x, const void* y) {
    const obj_string_t* str1 = *(const obj_string_t**)x;
    const obj_string_t* str2 = *(const obj_string_t**)y;
    return strcmp( str1->data, str2->data );
}

bool print_value(value_t val) {
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
        case TYPE_OBJECT: {
            switch (val.obj->type) {
                case OBJ_ARRAY: {
                    obj_array_t* array = AS_ARRAY(val);
                    printf("[");
                    for (size_t i = 0; i < array->size; ++ i) {
                        print_value(array->values[i]);
                        if (i != array->size - 1) {
                            printf(", ");
                        }
                    }
                    printf("]");
                    break;
                }
                case OBJ_INSTANCE: {
                    obj_instance_t* instance = AS_INSTANCE(val);
                    obj_string_t** fields = malloc(instance->class->size * sizeof(*fields));
                    for (size_t i = 0; i < instance->class->size; ++ i) {
                        fields[i] = instance->class->fields[i];
                    }
                    qsort(fields, instance->class->size, sizeof(*fields), compare_str_pointers);
                    printf("object(");
                    if (!IS_NULL(instance->extends)) {
                        printf("..=");
                        print_value(instance->extends);
                        if (instance->class->size != 0) {
                            printf(", ");
                        }
                    }
                    for (size_t i = 0; i < instance->class->size; ++ i) {
                        value_t val;
                        hash_map_fetch(&instance->fields, fields[i], &val);
                        printf("%s=", fields[i]->data);
                        print_value(val);
                        if (i != instance->class->size - 1) {
                            printf(", ");
                        }
                    }
                    printf(")");
                    break;
                }
            }
        }
            break;
        default:
            fprintf(stderr, "Uknown value type to print.\n");
            return false;
    }
    return true;
}

#define READ_BYTE_IP(vm) (*((vm)->ip++))
#define READ_WORD_IP(vm) ((vm)->ip += 2, (*((vm)->ip - 2) | (*((vm)->ip - 1) << 8)))

bool interpret_print(vm_t* vm) {
    // For some stupid reason, the first popped value should be printed last.
    // Yes, it is very stupid. The person who thought of this is bad and should feel bad.
    value_t obj = vm->bytecode.pool.data[READ_WORD_IP(vm)];
    if (!IS_STRING(obj)) {
        fprintf(stderr, "Print keyword accepts only string as it's first argument.\n");
        return false;
    }
    const char* str = AS_CSTRING(obj);
    int16_t index = READ_BYTE_IP(vm);
    vm->op_stack.size -= index;
    int16_t i = 0;
    for(const char* ptr = str; *ptr != '\0'; ptr ++) {
        if (*ptr == '~') {
            value_t val = vm->op_stack.data[vm->op_stack.size + i];
            if (!print_value(val)) {
                return false;
            }
            i += 1;
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

    if (index != i) {
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
    value_t fun;
    hash_map_fetch(&vm->global_var, name, &fun);
    return AS_FUNCTION(fun); 
}


value_t dispatch_builtin(obj_string_t* method_name, value_t receiver, value_t right_side, value_t right_right_side) {
#define CMP(x, y, z) (strcmp(x, y) == 0 || strcmp(x, z) == 0)
    if (IS_NUMBER(receiver)) {
        if (CMP(method_name->data, "+", "add")) {
            return INTEGER_VAL(receiver.num + right_side.num);
        } else if (CMP(method_name->data, "-", "sub")) {
            return INTEGER_VAL(receiver.num - right_side.num);
        } else if (CMP(method_name->data, "*", "mul")) {
            return INTEGER_VAL(receiver.num * right_side.num);
        } else if (CMP(method_name->data, "/", "div")) {
            return INTEGER_VAL(receiver.num / right_side.num);
        } else if (CMP(method_name->data, "%", "mod")) {
            return INTEGER_VAL(receiver.num % right_side.num);
        } else if (CMP(method_name->data, "<=", "le")) {
            return BOOL_VAL(IS_NUMBER(right_side) && receiver.num <= right_side.num);
        } else if (CMP(method_name->data, ">=", "ge")) {
            return BOOL_VAL(IS_NUMBER(right_side) && receiver.num >= right_side.num);
        } else if (CMP(method_name->data, "<", "lt")) {
            return BOOL_VAL(IS_NUMBER(right_side) && receiver.num < right_side.num);
        } else if (CMP(method_name->data, ">","gt")) {
            return BOOL_VAL(IS_NUMBER(right_side) && receiver.num > right_side.num);
        } else if (CMP(method_name->data, "==", "eq")) {
            return BOOL_VAL(IS_NUMBER(right_side) && receiver.num == right_side.num);
        } else if (CMP(method_name->data, "!=", "neq")) {
            return BOOL_VAL(!IS_NUMBER(right_side) || receiver.num != right_side.num);
        }
    } else if (IS_NULL(receiver)) {
        if (CMP(method_name->data, "==", "eq")) {
            return BOOL_VAL(IS_NULL(right_side));
        } else if (CMP(method_name->data, "!=", "neq")) {
            return BOOL_VAL(!IS_NULL(right_side));
        }
    } else if (IS_ARRAY(receiver)) {
        obj_array_t* arr = AS_ARRAY(receiver);
        if (CMP(method_name->data, "set", "set")) {
            arr->values[right_right_side.num] = right_side;
            return right_side;
        } else if (CMP(method_name->data, "get", "get")) {
            return arr->values[right_side.num];
        }
    } else if (IS_BOOL(receiver)) {
        if (CMP(method_name->data, "|", "or")) {
            return BOOL_VAL(receiver.b || right_side.b);
        } else if (CMP(method_name->data, "&", "and")) {
            return BOOL_VAL(receiver.b && right_side.b);
        } else if (CMP(method_name->data, "==", "eq")) {
            return BOOL_VAL(receiver.b == right_side.b);
        } else if (CMP(method_name->data, "!=", "neq")) {
            return BOOL_VAL(receiver.b != right_side.b);
        }
    }
    fprintf(stderr, "Unknown operator '%s' to dispatch.\n", method_name->data);
    exit(63);
#undef CMP
}

/// Recursively traverses instance and its parent classes to find field.
value_t find_field(value_t ins, obj_string_t* field_name) {
    if (!IS_INSTANCE(ins)) {
        fprintf(stderr, "Unknown field '%s'.", field_name->data);
        exit(123);
    }
    value_t field_val;
    if(!hash_map_fetch(&AS_INSTANCE(ins)->fields, field_name, &field_val)) {
        return find_field(AS_INSTANCE(ins)->extends, field_name);
    }
    return field_val;
}

void set_field(value_t ins, obj_string_t* field_name, value_t new_val) {
    if (!IS_INSTANCE(ins)) {
        fprintf(stderr, "Unknown field '%s'.", field_name->data);
        exit(123);
    }
    if(!hash_map_update(&AS_INSTANCE(ins)->fields, field_name, new_val)) {
        set_field(AS_INSTANCE(ins)->extends, field_name, new_val);
    }
}

interpret_result_t interpret(vm_t* vm)
{
    push_frame(&vm->frames, NULL);
    for (;(size_t)(vm->ip - vm->bytecode.bytecode) < vm->bytecode.size;) {
#ifdef __DEBUG__
        dissasemble_instruction(&vm->bytecode, vm->ip - vm->bytecode.bytecode);
        puts("");
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
            case OP_OBJECT: {
                obj_class_t* class = AS_CLASS(vm->bytecode.pool.data[READ_WORD_IP(vm)]);
                hash_map_t fields;
                init_hash_map(&fields);
                for (ssize_t i = class->size - 1; i >= 0; -- i) {
                    hash_map_insert(&fields, class->fields[i], pop(&vm->op_stack));
                }
                value_t instance = OBJ_INSTANCE_VAL(class, fields, pop(&vm->op_stack));
                push(&vm->op_stack, instance);
                break;
            }
            case OP_GET_FIELD: {
                obj_string_t* field_name = AS_STRING(vm->bytecode.pool.data[READ_WORD_IP(vm)]);
                push(&vm->op_stack, find_field(pop(&vm->op_stack), field_name));
                break;
            }
            case OP_SET_FIELD: {
                obj_string_t* field_name = AS_STRING(vm->bytecode.pool.data[READ_WORD_IP(vm)]);
                value_t val = pop(&vm->op_stack);
                value_t instance = pop(&vm->op_stack);
                set_field(instance, field_name, val);
                push(&vm->op_stack, val);
                break;
            }
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
                uint16_t index = READ_WORD_IP(vm);
                obj_string_t* method_name = AS_STRING(vm->bytecode.pool.data[index]);
                int args_cnt = READ_BYTE_IP(vm);

                value_t walk = vm->op_stack.data[vm->op_stack.size - args_cnt];
                value_t method;
                // Method dispatch, walk the inheritance tree and try finding the called method.
                // If primitive object is the parent, then try to call the builtin method.
                for (;;) {
                    if (IS_INSTANCE(walk)) {
                        if (!hash_map_fetch(&AS_INSTANCE(walk)->class->methods, method_name, &method)) {
                            walk = AS_INSTANCE(walk)->extends;
                        } else {
                            obj_function_t *func = AS_FUNCTION(method);
                            interpret_function_call(vm, func, args_cnt);
                            break;
                        }
                    } else {
                        value_t result = dispatch_builtin(method_name, walk, pop(&vm->op_stack), args_cnt == 2 ? NULL_VAL : pop(&vm->op_stack));
                        pop(&vm->op_stack); // Pop one more for the receiver
                        push(&vm->op_stack, result);
                        break;
                    }
                } 
                break;
            }
            default:
                fprintf(stderr, "Unknown instruction to interpret.\n");
                return INTERPRET_RUNTIME_ERROR;

        }
#ifdef __DEBUG__
        dissasemble_stack(&vm->op_stack);
#endif //__DEBUG__
    }
    return INTERPRET_OK;
}

#undef READ_BYTE_IP
#undef READ_WORD_IP

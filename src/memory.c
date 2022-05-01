#include <memory.h>
#include "include/memory.h"
#include "include/objects.h"
#include "include/vm.h"
#include "include/constant.h"
#include "include/buddy_alloc.h"

static void mark_object(obj_t* obj, vm_t* vm) {
    // Check that we don't visit already visited object
    if (obj != NULL && !obj->marked) {
        obj->marked = true;

        if (vm->gray_cnt >= vm->gray_capacity) {
            vm->gray_capacity = NEW_CAPACITY(vm->gray_capacity);
            vm->gray_stack = realloc(vm->gray_stack, sizeof(*vm->gray_stack) * vm->gray_capacity);
        }
    }
}

static void mark_val(value_t val, vm_t* vm) {
    // POD are not allocated on heap
    if (IS_OBJ(val)) {
        mark_object(AS_OBJ(val), vm);
    }
}

static void mark_roots(vm_t* vm) {
    for (size_t i = 0; i < vm->op_stack.size; ++i) {
        mark_val(vm->op_stack.data[i], vm);
    }
}

static void mark_table(hash_map_t* table, vm_t* vm) {
    for (size_t i = 0; i < table->capacity; ++i) {
        entry_t* entry = &table->entries[i];
        mark_object((obj_t*)entry->key, vm);
        mark_val(entry->value, vm);
    }
}

static void blacken(obj_t* obj, vm_t* vm) {
    switch (obj->type) {
        case OBJ_STRING:
        case OBJ_NATIVE:
        case OBJ_SLOT:
        // Function internals live in constant pool
        case OBJ_FUNCTION:
            break;
        case OBJ_INSTANCE: {
            obj_instance_t* i = (obj_instance_t*)obj;
            mark_object((obj_t*)i->class, vm);
            mark_val(i->extends, vm);
            mark_table(&i->fields, vm);
            break;
        }
        case OBJ_ARRAY: {
            obj_array_t* arr = (obj_array_t*)obj;
            mark_object(&arr->obj, vm);
            for (size_t i = 0; i < arr->size; ++i) {
                mark_val(arr->values[i], vm);
            }
            break;
        }
        case OBJ_CLASS: {
            obj_class_t* class = (obj_class_t*)obj;
            for (size_t i = 0; i < class->size; ++i) {
                mark_object(&class->fields[i]->obj, vm);
            }
            mark_table(&class->methods, vm);
            break;
        }
        default:
            fprintf(stderr, "Unknown object type in GC");
            exit(21);
    }
}

static void trace_references(vm_t* vm) {
    while (vm->gray_cnt > 0) {
        obj_t* obj = vm->gray_stack[--vm->gray_cnt];
        blacken(obj, vm);
    }
}

static void sweep(vm_t* vm) {
    // Helper previous node to keep the object list
    obj_t* prev = NULL;
    obj_t* obj = vm->objects;
    while (obj != NULL) {
        // Do not sweep marked, just move in the list
        if (obj->marked) {
            obj->marked = false;
            prev = obj;
            obj = obj->next;
        } else {
            obj_t* white = obj;
            obj = obj->next;
            // If prev is not null it's the already processed part
            // of the list, so just append the current (next) object to it
            if (prev != NULL) {
                prev->next = obj;
            // We're at the beginning, so replace the old list start with
            // new one.
            } else {
                vm->objects = obj;
            }
            heap_free(white);
        }
    }
}

void run_gc(vm_t* vm) {
#ifdef __DEBUG_GC__
    fputs(stderr, "-- GC start --\n");
#endif
    mark_roots(vm);
    mark_table(&vm->global_var, vm);
    // Mark local variables in each frame
    for (size_t i = 0; i < vm->frames.length; ++i) {
        for (size_t j = 0; j < MAX_LOCALS; ++ j) {
            mark_val(vm->frames.frames[i].locals_vector[j], vm);
        }
    }
    trace_references(vm);
    sweep(vm);
#ifdef __DEBUG_GC__
    fputs(stderr, "-- GC end --\n");
#endif
}

void* alloc_with_gc(size_t size, vm_t* vm) {
    void* ptr = heap_alloc(size);
    if (ptr == NULL) {
        // Try to run gc
        run_gc(vm);
        ptr = heap_alloc(size);
        // If even then the allocation failed, just die
        if (ptr == NULL) {
            fprintf(stderr, "The heap is not big enough to allocate object of size %lu\n", size);
            exit(11);
        }
    }
    return ptr;
}

void* realloc_with_gc(void* ptr, size_t size, vm_t* vm) {
    void* ret = heap_realloc(ptr, size);
    if (ret == NULL && size != 0) {
        run_gc(vm);
        ret = heap_realloc(ptr, size);
        if (ret == NULL) {
            fprintf(stderr, "The heap is not big enough to allocate object of size %lu\n", size);
            exit(11);
        }
    }
    return ret;
}

void* calloc_with_gc(size_t size, size_t cnt, vm_t* vm) {
    void* ret = alloc_with_gc(size * cnt, vm);
    memset(ret, 0, size * cnt);
    return ret;
}

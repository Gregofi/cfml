#include <memory.h>
#include "include/vm.h"
#include "include/constant.h"

static void mark_object(obj_t* obj) {
    if (obj != NULL) {
        obj->marked = true;
    }
}

static void mark_val(value_t val) {
    // POD are not allocated on heap
    if (IS_OBJ(val)) {
        mark_object(AS_OBJ(val));
    }
}

static void mark_roots(vm_t* vm) {
    for (size_t i = 0; i < vm->op_stack.size; ++i) {
        mark_val(vm->op_stack.data[i]);
    }
}

void run_gc(vm_t* vm) {
#ifdef __DEBUG_GC__
    fputs(stderr, "-- GC start --\n");
#endif
    mark_roots(vm);
#ifdef __DEBUG_GC__
    fputs(stderr, "-- GC end --\n");
#endif
}

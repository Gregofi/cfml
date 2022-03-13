#pragma once

#include "include/constant.h"
#include "include/bytecode.h"
#include "include/vm.h"

#include <stddef.h>
#include <stdio.h>

typedef enum {
    CD_INTEGER = 0x00,
    CD_NULL = 0x01,
    CD_STRING = 0x02,
    CD_METHOD = 0x03,
    CD_SLOT = 0x04,
    CD_CLASS = 0x05,
    CD_BOOLEAN = 0x06,
} constant_serialization_type_t;

void parse(vm_t* vm, const char* filename);

#pragma once

#define ARRAY_GROW_FACTOR 2
#define INIT_ARRAY_SIZE 8
#define NEW_CAPACITY(cap) ((cap) == 0 ? (INIT_ARRAY_SIZE) : (cap) * (ARRAY_GROW_FACTOR))

#define READ_4BYTES(value) (*((uint32_t*)((value))))
#define READ_2BYTES(value) (*((uint16_t*)((value))))
#define READ_BYTE(value) (*((uint8_t*)((value))))

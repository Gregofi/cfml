#pragma once

#define ARRAY_GROW_FACTOR 2
#define INIT_ARRAY_SIZE 8
#define NEW_CAPACITY(cap) ((cap) == 0 ? (INIT_ARRAY_SIZE) : (cap) * (ARRAY_GROW_FACTOR))

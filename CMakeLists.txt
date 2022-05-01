cmake_minimum_required(VERSION 3.20.5)

project(fml C)

set(CMAKE_C_COMPILER gcc)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${PROJECT_SOURCE_DIR}")

set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -pedantic -g -fsanitize=address -D__DEBUG_GC__  -D__STRESS_GC__")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_executable(fml main.c src/serializer.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c src/buddy_alloc.c src/hashmap.c)

enable_testing()

add_executable(interpreter_test tests/interpreter_test.c src/serializer.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c src/buddy_alloc.c src/hashmap.c)
add_executable(buddy_alloc_test tests/buddy_alloc_test.c src/serializer.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c src/buddy_alloc.c src/hashmap.c)
add_executable(hashmap_test tests/parser_test.c src/serializer.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c src/buddy_alloc.c src/hashmap.c)
add_executable(parser_test tests/interpreter_test.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c src/buddy_alloc.c src/hashmap.c)

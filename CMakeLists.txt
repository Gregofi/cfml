cmake_minimum_required(VERSION 3.20.5)

project(fml C)

set(CMAKE_C_COMPILER clang)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${PROJECT_SOURCE_DIR}")

set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -pedantic -g -fsanitize=address -D__DEBUG__")
set(CMAKE_C_FLAGS_RELEASE "-O2")

add_executable(fml main.c src/serializer.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c src/buddy_alloc.c src/hashmap.c)

enable_testing()

add_executable(interpreter_test tests/interpreter_test.c src/dissasembler.c src/serializer.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c src/buddy_alloc.c src/hashmap.c)
add_executable(buddy_alloc_test tests/buddy_alloc_test.c src/dissasembler.c src/buddy_alloc.c)
add_executable(hashmap_test tests/hashmap_test.c src/dissasembler.c src/constant.c src/hashmap.c src/buddy_alloc.c)
add_executable(parser_test tests/parser_test.c src/dissasembler.c src/constant.c src/hashmap.c src/serializer.c src/vm.c src/bytecode.c src/buddy_alloc.c)

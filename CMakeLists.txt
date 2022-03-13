cmake_minimum_required(VERSION 3.20.5)

project(fml C)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -g -I${PROJECT_SOURCE_DIR}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(fml main.c src/serializer.c src/bytecode.c src/dissasembler.c src/memory.c src/vm.c src/constant.c)

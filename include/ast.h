#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * Immutable dynamically allocated string.
 *
 * Not a flexible member due to being nested
 * in other structs.
 */
typedef struct {
    size_t size;
    char* data;
} string_t;

string_t create_string(const char* s) {
    size_t len = strlen(s);
    char* str = malloc(len);
    return (string_t){.size = len, .data = str};
}

typedef enum {
    AST_LITERAL,
    AST_VAR_DEF,
    AST_VAR_ACCESS,
    AST_VAR_ASSIGN,
    AST_ARRAY_DEF,
    AST_ARRAY_ACCESS,
    AST_ARRAY_ASSIGN,
    AST_FUNCTION,
    AST_FUNCTION_APPLY,
    AST_PRINT,
    AST_BLOCK,
    AST_TOP,
    AST_LOOP,
    AST_CONDITIONAL,
    AST_OBJECT_DEF,
    AST_FIELD_DEF,
    AST_FIELD_ASSIGN,
    AST_FIELD_ACCESS,
    AST_METHOD_DEF,
    AST_METHOD_CALL,
} ast_type_t;

typedef struct {
    ast_type_t type;
} ast_t;

typedef struct {
    size_t size;
    ast_t** data;
} ast_vec_t;

typedef enum {
    LIT_INT,
    LIT_BOOL,
    LIT_NULL,
} ast_literal_type_t;

/**
 * Represents integer, boolean or null literal.
 * To access respective type, cast data to either
 * int or boolean.
 */
typedef struct {
    ast_t ast;
    ast_literal_type_t type;
    int int_value;
} ast_literal_t;

/**
 * Variable { name: Identifier, value: AST }
 */
typedef struct {
    ast_t ast;
    ast_t* value;
    string_t identifier;
} ast_var_def_t;

/**
 * Variable Access { name: Identifier }
 */
typedef struct {
    ast_t ast;
    string_t identifier;
} ast_var_access_t;

/**
 * Variable assignment { name: Identifier, value: AST }
 */
typedef struct {
    ast_t ast;
    string_t identifier;
    ast_t* value;
} ast_var_assign_t;

/**
 * Array Definition { size: AST, value: AST }
 */
typedef struct {
    ast_t ast;
    ast_t* size;
    ast_t* value;
} ast_array_def_t;

/**
 * Array access { array: AST, index: AST }
 */
typedef struct {
    ast_t ast;
    ast_t* array;
    ast_t* index;
} ast_array_access_t;

/**
 * Array assignment { array: AST, index: AST, value: AST }
 */
typedef struct {
    ast_t ast;
    ast_t* array;
    ast_t* index;
    ast_t* value;
} ast_array_assign_t;

/**
 * Function { name: Identifier, parameters: Vec<Identifier>, body: AST }
 */
typedef struct {
    ast_t ast;
    string_t name;
    ast_t* body;
    size_t parameters_cnt;
    string_t parameters[];
} ast_function_t;

typedef struct {
    ast_t ast;
    string_t name;
    ast_vec_t args;
} ast_function_apply_t;

typedef struct {
    ast_t ast;
    string_t format;
    ast_vec_t args;
} ast_print_t;

typedef struct {
    ast_t ast;
    ast_vec_t body;
} ast_block_t;

typedef struct {
    ast_t ast;
    ast_vec_t body;
} ast_top_t;

typedef struct {
    ast_t ast;
    ast_t* condition;
    ast_t* body;
} ast_loop_t;

typedef struct {
    ast_t ast;
    ast_t* condition;
    ast_t* if_body;
    ast_t* else_body;
} ast_conditional_t;

typedef struct {
    ast_t ast;
    ast_vec_t members;
} ast_object_def_t;

typedef struct {
    ast_t ast;
    ast_t* object;
    string_t identifier;
    ast_t* value;
} ast_field_assign_t;

typedef struct {
    ast_t ast;
    ast_t* object;
    string_t identifier;
} ast_field_access_t;

typedef struct {
    ast_t ast;
    ast_t* object;
    string_t identifier;
    ast_vec_t arguments;
} ast_method_call_t;

#define BUILD_AST(t) (ast_t){.type = t}


/**
 * Creates new node representing literal
 * @param type - Type of the literal, either int, bool or null
 * @param val - int or boolean value. If type is null, val can be whatever.
 */
ast_literal_t* build_ast_int_lit(ast_literal_type_t type, int val);

ast_var_def_t* build_ast_var_def(ast_t* value, const char* identifier);

ast_var_access_t* build_ast_var_access(const char* identifier);

ast_var_assign_t* build_ast_var_assign(const char* identifier, ast_t* value);

ast_array_def_t* build_ast_array_def(ast_t* size, ast_t* value);

ast_array_access_t* build_ast_array_access(ast_t* array, ast_t* index);

ast_array_assign_t* build_ast_array_assign(ast_t* array, ast_t* index, ast_t* value);

ast_function_t* build_ast_function(const char* name, ast_t* body, size_t params_cnt, const char *params[]);

ast_function_apply_t* build_ast_function_apply(const char* name, ast_vec_t args);

ast_print_t* build_ast_print(const char* format, ast_vec_t args);

ast_block_t* build_ast_block(ast_vec_t stmts);

ast_top_t* build_ast_top(ast_vec_t body);

ast_loop_t* build_ast_loop(ast_t* cond, ast_t* body);

ast_conditional_t* build_ast_conditional(ast_t* cond, ast_t* if_body, ast_t* else_body);

ast_object_def_t* build_ast_object_def(ast_vec_t members);

ast_field_assign_t* build_ast_field_assign(ast_t* object, const char* identifier, ast_t* value);

ast_field_access_t* build_ast_field_access(ast_t* object, const char* identifier);

ast_method_call_t* build_ast_method_call(ast_t* object, const char* identifier, ast_vec_t args);

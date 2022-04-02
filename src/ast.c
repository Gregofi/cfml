#include "include/ast.h"

ast_literal_t* build_ast_int_lit(ast_literal_type_t type, int val) {
    ast_literal_t* lit = malloc(sizeof(*lit));
    lit->ast = BUILD_AST(AST_LITERAL);
    lit->type = type;
    lit->int_value = val;
    return lit;
}

ast_var_def_t* build_ast_var_def(ast_t* value, const char* identifier) {
    ast_var_def_t* var_def = malloc(sizeof(*var_def));
    var_def->ast = BUILD_AST(AST_VAR_DEF);
    var_def->identifier = create_string(identifier);
    var_def->value = value;
    return var_def;
}

ast_var_access_t* build_ast_var_access(const char* identifier) {
    ast_var_access_t* var_access = malloc(sizeof(*var_access));
    *var_access = (ast_var_access_t){
        .ast = BUILD_AST(AST_VAR_ACCESS),
        .identifier = create_string(identifier)
    };
    return var_access;
}

ast_var_assign_t* build_ast_var_assign(const char* identifier, ast_t* value) {
    ast_var_assign_t* var_assign = malloc(sizeof(*var_assign));
    *var_assign = (ast_var_assign_t){
        .ast = BUILD_AST(AST_VAR_ACCESS),
        .identifier = create_string(identifier),
        .value = value,
    };
    return var_assign;
}

ast_array_def_t* build_ast_array_def(ast_t* size, ast_t* value) {
    ast_array_def_t* array_def = malloc(sizeof(*array_def));
    *array_def = (ast_array_def_t){
        .ast = BUILD_AST(AST_ARRAY_DEF),
        .size = size,
        .value = value
    };
    return array_def;
}

ast_array_access_t* build_ast_array_access(ast_t* array, ast_t* index) {
    ast_array_access_t* array_access = malloc(sizeof(*array_access));
    *array_access = (ast_array_access_t){
        .ast = BUILD_AST(AST_ARRAY_ACCESS),
        .array = array,
        .index = index,
    };
    return array_access;
}

ast_array_assign_t* build_ast_array_assign(ast_t* array, ast_t* index, ast_t* value) {
    ast_array_assign_t* array_assign = malloc(sizeof(*array_assign));
    *array_assign = (ast_array_assign_t){
        .ast = BUILD_AST(AST_ARRAY_ASSIGN),
        .array = array,
        .index = index,
        .value = value,
    };
    return array_assign;
}

ast_function_t* build_ast_function(const char* name, ast_t* body, size_t params_cnt, const char *params[]) {

    ast_function_t* function = malloc(sizeof(*function) + params_cnt * sizeof(function->parameters[0]));
    *function = (ast_function_t){
        .ast = BUILD_AST(AST_FUNCTION),
        .name = create_string(name),
        .body = body,
        .parameters_cnt = params_cnt,
    };
    for (size_t i = 0; i < params_cnt; ++ i) {
        function->parameters[i] = create_string(params[i]);
    }
    return function;
}

ast_function_apply_t* build_ast_function_apply(const char* name, ast_vec_t args) {
    ast_function_apply_t* function_apply = malloc(sizeof(*function_apply));
    *function_apply = (ast_function_apply_t){
        .ast = BUILD_AST(AST_FUNCTION_APPLY),
        .name = create_string(name),
        .args = args,
    };
    return function_apply;
}

ast_print_t* build_ast_print(const char* format, ast_vec_t args) {
    ast_print_t* print = malloc(sizeof(*print));
    *print = (ast_print_t){
        .ast = BUILD_AST(AST_PRINT),
        .args = args,
        .format = create_string(format),
    };
    return print;
}

ast_block_t* build_ast_block(ast_vec_t stmts) {
    ast_block_t* block = malloc(sizeof(*block));
    *block = (ast_block_t){
        .ast = BUILD_AST(AST_BLOCK),
        .body = stmts,
    };
    return block;
}

ast_top_t* build_ast_top(ast_vec_t body) {
    ast_top_t* top = malloc(sizeof(*top));
    *top = (ast_top_t){
        .ast = BUILD_AST(AST_TOP),
        .body = body,
    };
    return top;
}

ast_loop_t* build_ast_loop(ast_t* cond, ast_t* body) {
    ast_loop_t* loop = malloc(sizeof(*loop));
    *loop = (ast_loop_t){
        .ast = BUILD_AST(AST_LOOP),
        .body = body,
        .condition = cond,
    };
    return loop;
}

ast_conditional_t* build_ast_conditional(ast_t* cond, ast_t* if_body, ast_t* else_body) {
    ast_conditional_t* conditional = malloc(sizeof(*conditional));
    *conditional = (ast_conditional_t){
        .ast = BUILD_AST(AST_CONDITIONAL),
        .condition = cond,
        .if_body = if_body,
        .else_body = else_body,
    };
    return conditional;
}

ast_object_def_t* build_ast_object_def(ast_vec_t members) {
    ast_object_def_t* object_def = malloc(sizeof(*object_def));
    *object_def = (ast_object_def_t){
        .ast = BUILD_AST(AST_OBJECT_DEF),
        .members = members,
    };
    return object_def;
}

ast_field_assign_t* build_ast_field_assign(ast_t* object, const char* identifier, ast_t* value) {
    ast_field_assign_t* field_assign = malloc(sizeof(*field_assign));
    *field_assign = (ast_field_assign_t){
        .ast = BUILD_AST(AST_FIELD_ASSIGN),
        .identifier = create_string(identifier),
        .object = object,
        .value = value,
    };
    return field_assign;
}

ast_field_access_t* build_ast_field_access(ast_t* object, const char* identifier) {
    ast_field_access_t* field_access = malloc(sizeof(*field_access));
    *field_access = (ast_field_access_t){
        .ast = BUILD_AST(AST_FIELD_ACCESS),
        .object = object,
        .identifier = create_string(identifier),
    };
    return field_access;
}

ast_method_call_t* build_ast_method_call(ast_t* object, const char* identifier, ast_vec_t args) {
    ast_method_call_t* method_call = malloc(sizeof(*method_call));
    *method_call = (ast_method_call_t){
        .ast = BUILD_AST(AST_METHOD_CALL),
        .object = object,
        .identifier = create_string(identifier),
        .arguments = args,
    };
    return method_call;
}

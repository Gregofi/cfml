#include <time.h>
#include "asserts.h"
#include "include/hashmap.h"
#include "include/constant.h"

TEST(basicTest) {
    hash_map_t hm;
    init_hash_map(&hm);
    value_t a = {.num = 1},b = {.num = 2},c = {.num = 3};
    
    obj_string_t* str1 = build_obj_string(4, "abcd", hash_string("abcd"));
    obj_string_t* str2 = build_obj_string(4, "xyz", hash_string("xyz"));
    obj_string_t* str3 = build_obj_string(4, "uiop", hash_string("uiop"));
    hash_map_insert(&hm, str1, a);
    hash_map_insert(&hm, str2, b);
    hash_map_insert(&hm, str3, c);

    ASSERT_W(hm.count == 3);

    value_t res;
    ASSERT_W(hash_map_fetch(&hm, str1, &res));
    ASSERT_W(res.num == 1);
    ASSERT_W(hash_map_fetch(&hm, str2, &res));
    ASSERT_W(res.num == 2);
    ASSERT_W(hash_map_fetch(&hm, str3, &res));
    ASSERT_W(res.num == 3);

    ASSERT_W(hash_map_delete(&hm, str2));

    ASSERT_W(hash_map_fetch(&hm, str1, &res));
    ASSERT_W(res.num == 1);
    ASSERT_W(!hash_map_fetch(&hm, str2, &res));
    ASSERT_W(hash_map_fetch(&hm, str3, &res));
    ASSERT_W(res.num == 3);

    free(str1);
    free(str2);
    free(str3);
    free_hash_map(&hm);
    return EXIT_SUCCESS;
}

TEST(reallocationTest) {
    hash_map_t hm;
    srand(time(NULL));
    const int SIZE = 10000;
    const int STRING_SIZE = 15;
    value_t vals[SIZE];
    obj_string_t* strings[SIZE];
    for (size_t i = 0; i < SIZE; ++ i) {
        vals[i].num = rand();
        char* str = malloc(STRING_SIZE);
        for (size_t j = 0; j < STRING_SIZE - 1; ++ j) {
            str[j] = (rand() % 10) + '0';
        }
        str[STRING_SIZE - 1] = '\0';
        strings[i] = build_obj_string(STRING_SIZE, str, strlen(str));
        free(str);
    }

    for (size_t i = 0; i < SIZE; ++ i) {
        hash_map_insert(&hm, strings[i], vals[i]);
    }

    for (size_t i = 0; i < SIZE; ++ i) {
        value_t val;
        ASSERT_W(hash_map_fetch(&hm, strings[i], &val));
        ASSERT_W(val.num == vals[i].num);
    }

    for (size_t i = 0; i < SIZE; i += rand() % 10 + 1) {
        ASSERT_W(hash_map_delete(&hm, strings[i]));
        ASSERT_W(!hash_map_delete(&hm, strings[i]));
        vals[i].num = -1;
    }

    for (size_t i = 0; i < SIZE; ++ i) {
        value_t val;
        bool found = hash_map_fetch(&hm, strings[i], &val);
        if (found) {
            ASSERT_W(val.num == vals[i].num);
        } else {
            ASSERT_W(vals[i].num == -1);
        }
    }

    for (size_t i = 0; i < SIZE; ++ i) {
        free(strings[i]);
    }



    free_hash_map(&hm);
    return EXIT_SUCCESS;
}


int main(void) {
    RUN_TEST(basicTest);
    RUN_TEST(reallocationTest);
}

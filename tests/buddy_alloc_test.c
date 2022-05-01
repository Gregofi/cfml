#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "include/buddy_alloc.h"
#include "asserts.h"

TEST(basicTest) {
    uint8_t *p0, *p1, *p2;
    uint8_t* mem_pool = malloc(3 * 1048576);
    heap_init(mem_pool, 2097152, NULL);
    ASSERT_W((void*)(p0 = (uint8_t*)heap_alloc(1)) != NULL);
    memset(p0, 0, 1);
    ASSERT_W((void*)(p1 = (uint8_t*)heap_alloc(2)) != NULL);
    memset(p1, 0, 2);
    ASSERT_W((void*)(p2 = (uint8_t*)heap_alloc(64)) != NULL);
    memset(p2, 0, 64);
    ASSERT_W(heap_free(p0));
    ASSERT_W(heap_free(p1));
    ASSERT_W(heap_free(p2));
    ASSERT_W(heap_done() == 0);
    free(mem_pool);
    return EXIT_SUCCESS;
}

TEST(basicTest2) {
    uint8_t *p0, *p1, *p2, *p3, *p4;
    static uint8_t  mem_pool[3 * 1048576];

    heap_init(mem_pool, 2097152, NULL);
    ASSERT_W((void*)(p0 = (uint8_t*)heap_alloc(512000)) != NULL);
    memset(p0, 0, 512000);
    ASSERT_W((void*)(p1 = (uint8_t*)heap_alloc(511000)) != NULL);
    memset(p1, 0, 511000);
    ASSERT_W((void*)(p2 = (uint8_t*)heap_alloc(26000)) != NULL);
    memset(p2, 0, 26000);
    ASSERT_W(heap_done() == 3);

    heap_init(mem_pool, 2097152, NULL);
    ASSERT_W((void*)(p0 = (uint8_t*)heap_alloc(1000000)) != NULL);
    memset(p0, 0, 1000000);
    ASSERT_W((void*)(p1 = (uint8_t*)heap_alloc(250000)) != NULL);
    memset(p1, 0, 250000);
    ASSERT_W((void*)(p2 = (uint8_t*)heap_alloc(250000)) != NULL);
    memset(p2, 0, 250000);
    ASSERT_W((void*)(p3 = (uint8_t*)heap_alloc(250000)) != NULL);
    memset(p3, 0, 250000);
    ASSERT_W((void*)(p4 = (uint8_t*)heap_alloc(50000)) != NULL);
    memset(p4, 0, 50000);
    ASSERT_W(heap_free(p2));
    ASSERT_W(heap_free(p4));
    ASSERT_W(heap_free(p3));
    ASSERT_W(heap_free(p1));
    ASSERT_W((p1 = (uint8_t*)heap_alloc(500000)) != NULL);
    memset(p1, 0, 500000);
    ASSERT_W(heap_free(p0));
    ASSERT_W(heap_free(p1));

    ASSERT_W(heap_done() == 0);

    heap_init(mem_pool, 2359296, NULL);
    ASSERT_W((void*)(p0 = (uint8_t*)heap_alloc(1000000)) != NULL);
    memset(p0, 0, 1000000);
    ASSERT_W((void*)(p1 = (uint8_t*)heap_alloc(500000)) != NULL);
    memset(p1, 0, 500000);
    ASSERT_W((void*)(p2 = (uint8_t*)heap_alloc(500000)) != NULL);
    memset(p2, 0, 500000);
    ASSERT_W((void*)(p3 = (uint8_t*)heap_alloc(500000)) == NULL);
    ASSERT_W(heap_free(p2));
    ASSERT_W((void*)(p2 = (uint8_t*)heap_alloc(300000)) != NULL);
    memset(p2, 0, 300000);
    ASSERT_W(heap_free(p0));
    ASSERT_W(heap_free(p1));
    ASSERT_W(heap_done() == 1);

    heap_init(mem_pool, 2359296, NULL);
    ASSERT_W((void*)(p0 = (uint8_t*)heap_alloc(1000000)) != NULL);
    memset(p0, 0, 1000000);
    ASSERT_W(!heap_free(p0 + 1000));

    ASSERT_W(heap_done() == 1);
    return EXIT_SUCCESS;
}

TEST(extensiveTest) {
    static uint8_t  mem_pool[3 * 1048576];
    heap_init(mem_pool, 2359296, NULL);
    void* ptrs[10000];
    for (size_t i = 0; i < 10000; ++i) {
        int *p;
        ASSERT_W((p = (int*)heap_alloc(4)));
        *p = 1;
        ptrs[i] = p;
    }

    for (size_t i = 0; i < 10000; ++ i)
        ASSERT_W(*(int*)ptrs[i] == 1);

    for (size_t i = 0; i < 10000; ++ i)
        ASSERT_W(heap_free(ptrs[i]));
    ASSERT_W(heap_done() == 0);

    for (size_t i = 0; i < 10000; ++i) {
        int *p;
        ASSERT_W((p = (int*)heap_alloc(4)));
        *p = 2;
        ptrs[i] = p;
    }

    for (size_t i = 0; i < 10000; ++ i)
        ASSERT_W(*(int*)ptrs[i] == 2);

    for (size_t i = 0; i < 10000; ++ i)
        ASSERT_W(heap_free(ptrs[i]));
    ASSERT_W(heap_done() == 0);
    return EXIT_SUCCESS;
}

int main(void) {
    RUN_TEST(basicTest);
    RUN_TEST(basicTest2);
    RUN_TEST(extensiveTest);
}

#include "../tests/harness/unity.h"
#include "../src/lab.h"
#include <stdlib.h>
#include <string.h>

/* === Required by Unity === */
void setUp(void) {}
void tearDown(void) {}

/* === Helper structures and functions === */
typedef struct {
    int id;
    char name[50];
} TestObject;

static TestObject *create_test_object(int id, const char *name) {
    TestObject *obj = (TestObject *)malloc(sizeof(TestObject));
    if (obj) {
        obj->id = id;
        strncpy(obj->name, name, sizeof(obj->name) - 1);
        obj->name[sizeof(obj->name) - 1] = '\0';
    }
    return obj;
}

static void free_test_object(void *obj) {
    free(obj);
}

/* === Tests === */

void test_list_create_destroy(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    TEST_ASSERT_NOT_NULL(list);
    TEST_ASSERT_EQUAL_UINT32(0, list_size(list));
    TEST_ASSERT_TRUE(list_is_empty(list));
    list_destroy(list, NULL);

    /* Create and destroy with free_func */
    list = list_create(LIST_LINKED_SENTINEL);
    for (int i = 0; i < 5; i++) {
        list_append(list, create_test_object(i, "tmp"));
    }
    list_destroy(list, free_test_object);
}

void test_list_append_and_get(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    TestObject *obj1 = create_test_object(1, "One");
    TestObject *obj2 = create_test_object(2, "Two");
    TestObject *obj3 = create_test_object(3, "Three");

    TEST_ASSERT_TRUE(list_append(list, obj1));
    TEST_ASSERT_TRUE(list_append(list, obj2));
    TEST_ASSERT_TRUE(list_append(list, obj3));
    TEST_ASSERT_EQUAL_UINT32(3, list_size(list));

    TestObject *r1 = (TestObject *)list_get(list, 0);
    TestObject *r2 = (TestObject *)list_get(list, 1);
    TestObject *r3 = (TestObject *)list_get(list, 2);

    TEST_ASSERT_NOT_NULL(r1);
    TEST_ASSERT_NOT_NULL(r2);
    TEST_ASSERT_NOT_NULL(r3);
    TEST_ASSERT_EQUAL_INT(1, r1->id);
    TEST_ASSERT_EQUAL_INT(2, r2->id);
    TEST_ASSERT_EQUAL_INT(3, r3->id);

    list_destroy(list, free_test_object);
}

void test_list_insert(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    TestObject *o1 = create_test_object(1, "First");
    TestObject *o2 = create_test_object(2, "Second");
    TestObject *o3 = create_test_object(3, "Third");
    TestObject *o4 = create_test_object(4, "Fourth");

    /* Insert at index 0 (empty list) */
    TEST_ASSERT_TRUE(list_insert(list, 0, o1));

    /* Insert at end */
    TEST_ASSERT_TRUE(list_insert(list, 1, o3));

    /* Insert in middle */
    TEST_ASSERT_TRUE(list_insert(list, 1, o2));

    /* Insert at beginning */
    TEST_ASSERT_TRUE(list_insert(list, 0, o4));

    /* Verify order: 4,1,2,3 */
    TestObject *r0 = list_get(list, 0);
    TestObject *r1 = list_get(list, 1);
    TestObject *r2 = list_get(list, 2);
    TestObject *r3 = list_get(list, 3);
    TEST_ASSERT_EQUAL_INT(4, r0->id);
    TEST_ASSERT_EQUAL_INT(1, r1->id);
    TEST_ASSERT_EQUAL_INT(2, r2->id);
    TEST_ASSERT_EQUAL_INT(3, r3->id);

    /* Invalid insert */
    TestObject *bad = create_test_object(99, "Bad");
    TEST_ASSERT_FALSE(list_insert(list, 10, bad));
    free_test_object(bad);

    list_destroy(list, free_test_object);
}

void test_list_remove(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    TestObject *objs[4];
    for (int i = 0; i < 4; i++) {
        objs[i] = create_test_object(i + 1, "X");
        list_append(list, objs[i]);
    }

    /* Remove from middle */
    TestObject *r = list_remove(list, 1);
    TEST_ASSERT_EQUAL_INT(2, r->id);
    free_test_object(r);

    /* Remove from beginning */
    r = list_remove(list, 0);
    TEST_ASSERT_EQUAL_INT(1, r->id);
    free_test_object(r);

    /* Remove from end */
    r = list_remove(list, 1);
    TEST_ASSERT_EQUAL_INT(4, r->id);
    free_test_object(r);

    /* Remove last element */
    r = list_remove(list, 0);
    TEST_ASSERT_EQUAL_INT(3, r->id);
    free_test_object(r);

    /* Now empty */
    TEST_ASSERT_TRUE(list_is_empty(list));
    TEST_ASSERT_NULL(list_remove(list, 0));

    list_destroy(list, NULL);
}

void test_list_get_invalid(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    TEST_ASSERT_NULL(list_get(list, 0));
    list_destroy(list, NULL);
}

void test_null_list_operations(void) {
    TEST_ASSERT_FALSE(list_append(NULL, (void *)1));
    TEST_ASSERT_FALSE(list_insert(NULL, 0, (void *)1));
    TEST_ASSERT_NULL(list_remove(NULL, 0));
    TEST_ASSERT_NULL(list_get(NULL, 0));
    TEST_ASSERT_EQUAL_UINT32(0, list_size(NULL));
    TEST_ASSERT_TRUE(list_is_empty(NULL));
}

void test_large_list_and_circular_integrity(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    for (int i = 0; i < 1000; i++) {
        list_append(list, create_test_object(i, "bulk"));
    }
    TEST_ASSERT_EQUAL_UINT32(1000, list_size(list));

    /* Remove some elements */
    free_test_object(list_remove(list, 500));
    free_test_object(list_remove(list, 0));
    free_test_object(list_remove(list, list_size(list) - 1));

    /* Insert at front and back */
    list_insert(list, 0, create_test_object(2000, "front"));
    list_insert(list, list_size(list), create_test_object(3000, "back"));
    TEST_ASSERT_EQUAL_UINT32(999, list_size(list));

    /* Clear */
    while (!list_is_empty(list)) {
        free_test_object(list_remove(list, 0));
    }
    TEST_ASSERT_TRUE(list_is_empty(list));

    list_destroy(list, NULL);
}

// Test creating list with invalid type
void test_list_create_invalid_type(void) {
    List *list = list_create(999); // unsupported type
    TEST_ASSERT_NULL(list);
}

// Test destroying NULL list
void test_list_destroy_null(void) {
    // Should not crash
    list_destroy(NULL, NULL);
    TEST_ASSERT_TRUE(1); // dummy check just to satisfy Unity
}

// Test list_get on NULL list
void test_list_get_null(void) {
    TEST_ASSERT_NULL(list_get(NULL, 0));
    TEST_ASSERT_NULL(list_get(NULL, 100)); // redundant but forces both args
}

//P2
void test_compare_int_and_sort(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    // Add integers in random order
    int *a = malloc(sizeof(int)); *a = 5;
    int *b = malloc(sizeof(int)); *b = 10;
    int *c = malloc(sizeof(int)); *c = 3;
    int *d = malloc(sizeof(int)); *d = 8;

    list_append(list, a);
    list_append(list, b);
    list_append(list, c);
    list_append(list, d);

    // Sort entire list using compare_int (descending order)
    sort(list, 0, list_size(list) - 1, compare_int);

    TEST_ASSERT_TRUE(is_sorted(list, compare_int));

    int *v0 = list_get(list, 0);
    int *v1 = list_get(list, 1);
    int *v2 = list_get(list, 2);
    int *v3 = list_get(list, 3);

    TEST_ASSERT_EQUAL_INT(10, *v0);
    TEST_ASSERT_EQUAL_INT(8, *v1);
    TEST_ASSERT_EQUAL_INT(5, *v2);
    TEST_ASSERT_EQUAL_INT(3, *v3);

    list_destroy(list, free);
}

void test_compare_str_and_sort(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    char *s1 = strdup("banana");
    char *s2 = strdup("apple");
    char *s3 = strdup("cherry");

    list_append(list, s1);
    list_append(list, s2);
    list_append(list, s3);

    // Sort lexicographically
    sort(list, 0, list_size(list) - 1, compare_str);

    TEST_ASSERT_TRUE(is_sorted(list, compare_str));

    char *r0 = list_get(list, 0);
    char *r1 = list_get(list, 1);
    char *r2 = list_get(list, 2);

    TEST_ASSERT_EQUAL_STRING("apple", r0);
    TEST_ASSERT_EQUAL_STRING("banana", r1);
    TEST_ASSERT_EQUAL_STRING("cherry", r2);

    list_destroy(list, free);
}

void test_merge_lists(void) {
    List *l1 = list_create(LIST_LINKED_SENTINEL);
    List *l2 = list_create(LIST_LINKED_SENTINEL);

    int *a = malloc(sizeof(int)); *a = 10;
    int *b = malloc(sizeof(int)); *b = 8;
    int *c = malloc(sizeof(int)); *c = 5;
    list_append(l1, a);
    list_append(l1, b);
    list_append(l1, c);

    int *d = malloc(sizeof(int)); *d = 7;
    int *e = malloc(sizeof(int)); *e = 4;
    int *f = malloc(sizeof(int)); *f = 1;
    list_append(l2, d);
    list_append(l2, e);
    list_append(l2, f);

    // Both lists are already sorted descending
    List *merged = merge(l1, l2, compare_int);

    TEST_ASSERT_EQUAL_UINT32(6, list_size(merged));
    TEST_ASSERT_TRUE(is_sorted(merged, compare_int));

    int *first = list_get(merged, 0);
    int *last = list_get(merged, 5);
    TEST_ASSERT_EQUAL_INT(10, *first);
    TEST_ASSERT_EQUAL_INT(1, *last);

    list_destroy(l1, free);
    list_destroy(l2, free);
    list_destroy(merged, NULL); // merged reuses data pointers
}

void test_is_sorted_edge_cases(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);

    // Empty list -> sorted
    TEST_ASSERT_TRUE(is_sorted(list, compare_int));

    int *x = malloc(sizeof(int)); *x = 42;
    list_append(list, x);

    // Single element -> sorted
    TEST_ASSERT_TRUE(is_sorted(list, compare_int));

    // Add one more element, out of order
    int *y = malloc(sizeof(int)); *y = 99;
    list_append(list, y);

    TEST_ASSERT_FALSE(is_sorted(list, compare_int));

    list_destroy(list, free);
}

void test_randomized_sort_and_is_sorted(void) {
    List *list = list_create(LIST_LINKED_SENTINEL);
    srand(12345); // fixed seed for reproducibility

    // Fill with 100 random ints
    for (int i = 0; i < 100; i++) {
        int *val = malloc(sizeof(int));
        *val = rand() % 1000; 
        list_append(list, val);
    }

    // Sort entire list descending
    sort(list, 0, list_size(list) - 1, compare_int);

    // Verify it is sorted
    TEST_ASSERT_TRUE(is_sorted(list, compare_int));

    // Verify descending property explicitly
    for (size_t i = 1; i < list_size(list); i++) {
        int *prev = list_get(list, i - 1);
        int *cur  = list_get(list, i);
        TEST_ASSERT_TRUE(*prev >= *cur);
    }

    list_destroy(list, free);
}

void test_randomized_merge(void) {
    List *l1 = list_create(LIST_LINKED_SENTINEL);
    List *l2 = list_create(LIST_LINKED_SENTINEL);
    srand(54321); // fixed seed

    // Fill l1 with 50 random ints
    for (int i = 0; i < 50; i++) {
        int *val = malloc(sizeof(int));
        *val = rand() % 1000;
        list_append(l1, val);
    }
    // Fill l2 with 50 random ints
    for (int i = 0; i < 50; i++) {
        int *val = malloc(sizeof(int));
        *val = rand() % 1000;
        list_append(l2, val);
    }

    // Sort each individually
    sort(l1, 0, list_size(l1) - 1, compare_int);
    sort(l2, 0, list_size(l2) - 1, compare_int);

    // Merge them
    List *merged = merge(l1, l2, compare_int);

    // Check merged size
    TEST_ASSERT_EQUAL_UINT32(100, list_size(merged));

    // Check sortedness
    TEST_ASSERT_TRUE(is_sorted(merged, compare_int));

    // Spot check extremes
    int *first = list_get(merged, 0);
    int *last  = list_get(merged, list_size(merged) - 1);
    TEST_ASSERT_TRUE(*first >= *last);

    list_destroy(l1, free);
    list_destroy(l2, free);
    list_destroy(merged, NULL); // merged shares data pointers
}


/* === Test Runner === */
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_list_create_destroy);
    RUN_TEST(test_list_append_and_get);
    RUN_TEST(test_list_insert);
    RUN_TEST(test_list_remove);
    RUN_TEST(test_list_get_invalid);
    RUN_TEST(test_null_list_operations);
    RUN_TEST(test_large_list_and_circular_integrity);
    RUN_TEST(test_list_create_invalid_type);
    RUN_TEST(test_list_destroy_null);
    RUN_TEST(test_list_get_null);
    //P2
    RUN_TEST(test_compare_int_and_sort);
    RUN_TEST(test_compare_str_and_sort);
    RUN_TEST(test_merge_lists);
    RUN_TEST(test_is_sorted_edge_cases);
    RUN_TEST(test_randomized_sort_and_is_sorted);
    RUN_TEST(test_randomized_merge);

    return UNITY_END();
}



/*
 * Copyright 2025 Samuel "Lord-Windy" Brown
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <datazoo/pearl.h>
#include <samrena.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_pearl_empty_set_operations(void) {
    printf("Testing operations on empty set...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    assert(pearl_size(pearl) == 0);
    assert(pearl_is_empty(pearl) == true);
    
    int value = 42;
    assert(pearl_contains(pearl, &value) == false);
    assert(pearl_remove(pearl, &value) == false);
    assert(pearl_get_last_error(pearl) == PEARL_ERROR_ELEMENT_NOT_FOUND);
    
    pearl_clear(pearl);
    assert(pearl_size(pearl) == 0);
    assert(pearl_is_empty(pearl) == true);
    
    PearlStats stats = pearl_get_stats(pearl);
    assert(stats.total_operations >= 1);
    assert(stats.total_collisions == 0);
    assert(stats.max_chain_length == 0);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Empty set operations test passed\n");
}

static void test_pearl_single_element_operations(void) {
    printf("Testing operations with single element...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int value = 100;
    
    bool result = pearl_add(pearl, &value);
    assert(result == true);
    assert(pearl_size(pearl) == 1);
    assert(pearl_is_empty(pearl) == false);
    assert(pearl_contains(pearl, &value) == true);
    
    result = pearl_add(pearl, &value);
    assert(result == false);
    assert(pearl_size(pearl) == 1);
    assert(pearl_get_last_error(pearl) == PEARL_ERROR_ELEMENT_EXISTS);
    
    result = pearl_remove(pearl, &value);
    assert(result == true);
    assert(pearl_size(pearl) == 0);
    assert(pearl_is_empty(pearl) == true);
    assert(pearl_contains(pearl, &value) == false);
    
    result = pearl_remove(pearl, &value);
    assert(result == false);
    assert(pearl_get_last_error(pearl) == PEARL_ERROR_ELEMENT_NOT_FOUND);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Single element operations test passed\n");
}

static void test_pearl_zero_element_size(void) {
    printf("Testing creation with zero element size...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(0, 16, arena);
    assert(pearl == NULL);
    
    samrena_destroy(arena);
    printf("✓ Zero element size test passed\n");
}

static void test_pearl_minimum_capacity(void) {
    printf("Testing creation with very small capacity...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 1, arena);
    assert(pearl != NULL);
    
    int values[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        bool result = pearl_add(pearl, &values[i]);
        assert(result == true);
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    assert(pearl_size(pearl) == 5);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Minimum capacity test passed\n");
}

static void test_pearl_duplicate_bytes(void) {
    printf("Testing elements with identical byte patterns...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    typedef struct {
        int a;
        int b;
    } TestStruct;
    
    Pearl *pearl = pearl_create(sizeof(TestStruct), 16, arena);
    assert(pearl != NULL);
    
    TestStruct s1 = {1, 2};
    TestStruct s2 = {1, 2};
    TestStruct s3 = {2, 1};
    
    bool result = pearl_add(pearl, &s1);
    assert(result == true);
    assert(pearl_size(pearl) == 1);
    
    result = pearl_add(pearl, &s2);
    assert(result == false);
    assert(pearl_size(pearl) == 1);
    assert(pearl_get_last_error(pearl) == PEARL_ERROR_ELEMENT_EXISTS);
    
    result = pearl_add(pearl, &s3);
    assert(result == true);
    assert(pearl_size(pearl) == 2);
    
    assert(pearl_contains(pearl, &s1) == true);
    assert(pearl_contains(pearl, &s2) == true);
    assert(pearl_contains(pearl, &s3) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Duplicate bytes test passed\n");
}

static void test_pearl_large_element_size(void) {
    printf("Testing with large element size...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    typedef struct {
        char data[1024];
    } LargeStruct;
    
    Pearl *pearl = pearl_create(sizeof(LargeStruct), 16, arena);
    assert(pearl != NULL);
    
    LargeStruct large1, large2;
    memset(&large1, 0xAA, sizeof(LargeStruct));
    memset(&large2, 0xBB, sizeof(LargeStruct));
    
    bool result = pearl_add(pearl, &large1);
    assert(result == true);
    assert(pearl_contains(pearl, &large1) == true);
    assert(pearl_contains(pearl, &large2) == false);
    
    result = pearl_add(pearl, &large2);
    assert(result == true);
    assert(pearl_size(pearl) == 2);
    
    assert(pearl_contains(pearl, &large1) == true);
    assert(pearl_contains(pearl, &large2) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Large element size test passed\n");
}

static bool always_equal(const void *a, const void *b, size_t size) {
    (void)a; (void)b; (void)size;
    return true;
}

static uint32_t same_hash(const void *element, size_t size) {
    (void)element; (void)size;
    return 42;
}

static void test_pearl_custom_equality_function(void) {
    printf("Testing custom equality function...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_custom(sizeof(int), 16, arena, same_hash, always_equal);
    assert(pearl != NULL);
    
    int value1 = 10;
    int value2 = 20;
    
    bool result = pearl_add(pearl, &value1);
    assert(result == true);
    assert(pearl_size(pearl) == 1);
    
    result = pearl_add(pearl, &value2);
    assert(result == false);
    assert(pearl_get_last_error(pearl) == PEARL_ERROR_ELEMENT_EXISTS);
    assert(pearl_size(pearl) == 1);
    
    assert(pearl_contains(pearl, &value1) == true);
    assert(pearl_contains(pearl, &value2) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Custom equality function test passed\n");
}

static void test_pearl_operations_after_clear(void) {
    printf("Testing operations after clear...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int values[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    assert(pearl_size(pearl) == 5);
    
    pearl_clear(pearl);
    assert(pearl_size(pearl) == 0);
    assert(pearl_is_empty(pearl) == true);
    
    for (int i = 0; i < 5; i++) {
        assert(pearl_contains(pearl, &values[i]) == false);
    }
    
    for (int i = 0; i < 5; i++) {
        bool result = pearl_add(pearl, &values[i]);
        assert(result == true);
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    assert(pearl_size(pearl) == 5);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Operations after clear test passed\n");
}

static PearlError callback_error = PEARL_ERROR_NONE;
static const char *callback_message = NULL;

static void error_callback(PearlError error, const char *message, void *user_data) {
    (void)user_data;
    callback_error = error;
    callback_message = message;
}

static void test_pearl_error_callback(void) {
    printf("Testing error callback functionality...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    pearl_set_error_callback(pearl, error_callback, NULL);
    
    int value = 42;
    pearl_add(pearl, &value);
    
    callback_error = PEARL_ERROR_NONE;
    callback_message = NULL;
    
    bool result = pearl_add(pearl, &value);
    assert(result == false);
    assert(callback_error == PEARL_ERROR_ELEMENT_EXISTS);
    assert(callback_message != NULL);
    assert(strcmp(callback_message, "Element already exists") == 0);
    
    callback_error = PEARL_ERROR_NONE;
    callback_message = NULL;
    
    int non_existent = 999;
    result = pearl_remove(pearl, &non_existent);
    assert(result == false);
    assert(callback_error == PEARL_ERROR_ELEMENT_NOT_FOUND);
    assert(callback_message != NULL);
    assert(strcmp(callback_message, "Element not found") == 0);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Error callback test passed\n");
}

int main(void) {
    printf("=== Pearl Edge Cases Tests ===\n");
    
    test_pearl_empty_set_operations();
    test_pearl_single_element_operations();
    test_pearl_zero_element_size();
    test_pearl_minimum_capacity();
    test_pearl_duplicate_bytes();
    test_pearl_large_element_size();
    test_pearl_custom_equality_function();
    test_pearl_operations_after_clear();
    test_pearl_error_callback();
    
    printf("\n✅ All Pearl edge cases tests passed!\n");
    return 0;
}
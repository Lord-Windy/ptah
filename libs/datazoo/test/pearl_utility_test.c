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

static void test_pearl_copy(void) {
    printf("Testing Pearl copy...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena1 = samrena_create(&config);
    Samrena *arena2 = samrena_create(&config);
    assert(arena1 != NULL);
    assert(arena2 != NULL);
    
    Pearl *original = pearl_create(sizeof(int), 16, arena1);
    assert(original != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        assert(pearl_add(original, &values[i]) == true);
    }
    assert(pearl_size(original) == 5);
    
    Pearl *copy = pearl_copy(original, arena2);
    assert(copy != NULL);
    assert(pearl_size(copy) == pearl_size(original));
    
    for (int i = 0; i < 5; i++) {
        assert(pearl_contains(copy, &values[i]) == true);
    }
    
    int new_value = 60;
    assert(pearl_add(copy, &new_value) == true);
    assert(pearl_size(copy) == 6);
    assert(pearl_size(original) == 5);
    assert(pearl_contains(copy, &new_value) == true);
    assert(pearl_contains(original, &new_value) == false);
    
    pearl_destroy(original);
    pearl_destroy(copy);
    samrena_destroy(arena1);
    samrena_destroy(arena2);
    printf("✓ Pearl copy test passed\n");
}

static void test_pearl_copy_empty(void) {
    printf("Testing Pearl copy empty set...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena1 = samrena_create(&config);
    Samrena *arena2 = samrena_create(&config);
    assert(arena1 != NULL);
    assert(arena2 != NULL);
    
    Pearl *original = pearl_create(sizeof(int), 16, arena1);
    assert(original != NULL);
    assert(pearl_is_empty(original) == true);
    
    Pearl *copy = pearl_copy(original, arena2);
    assert(copy != NULL);
    assert(pearl_is_empty(copy) == true);
    assert(pearl_size(copy) == 0);
    
    pearl_destroy(original);
    pearl_destroy(copy);
    samrena_destroy(arena1);
    samrena_destroy(arena2);
    printf("✓ Pearl copy empty test passed\n");
}

static void test_pearl_to_array(void) {
    printf("Testing Pearl to array conversion...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        assert(pearl_add(pearl, &values[i]) == true);
    }
    
    int result_array[10];
    size_t count = pearl_to_array(pearl, result_array, 10);
    assert(count == 5);
    
    for (size_t i = 0; i < count; i++) {
        bool found = false;
        for (int j = 0; j < 5; j++) {
            if (result_array[i] == values[j]) {
                found = true;
                break;
            }
        }
        assert(found == true);
    }
    
    int small_array[3];
    count = pearl_to_array(pearl, small_array, 3);
    assert(count == 3);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl to array test passed\n");
}

static void test_pearl_to_array_empty(void) {
    printf("Testing Pearl to array empty set...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int result_array[10];
    size_t count = pearl_to_array(pearl, result_array, 10);
    assert(count == 0);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl to array empty test passed\n");
}

static void test_pearl_from_array(void) {
    printf("Testing Pearl from array creation...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    int values[] = {10, 20, 30, 40, 50, 20, 30};
    Pearl *pearl = pearl_from_array(values, 7, sizeof(int), arena);
    assert(pearl != NULL);
    
    assert(pearl_size(pearl) == 5);
    
    for (int i = 0; i < 5; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl from array test passed\n");
}

static void test_pearl_from_array_empty(void) {
    printf("Testing Pearl from empty array...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    int values[] = {10, 20, 30};
    Pearl *pearl = pearl_from_array(values, 0, sizeof(int), arena);
    assert(pearl == NULL);
    
    samrena_destroy(arena);
    printf("✓ Pearl from empty array test passed\n");
}

static void test_pearl_roundtrip(void) {
    printf("Testing Pearl array roundtrip...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena1 = samrena_create(&config);
    Samrena *arena2 = samrena_create(&config);
    assert(arena1 != NULL);
    assert(arena2 != NULL);
    
    int original_values[] = {1, 5, 10, 15, 20, 25, 30};
    Pearl *original = pearl_from_array(original_values, 7, sizeof(int), arena1);
    assert(original != NULL);
    assert(pearl_size(original) == 7);
    
    int extracted[10];
    size_t count = pearl_to_array(original, extracted, 10);
    assert(count == 7);
    
    Pearl *roundtrip = pearl_from_array(extracted, count, sizeof(int), arena2);
    assert(roundtrip != NULL);
    assert(pearl_size(roundtrip) == 7);
    
    for (int i = 0; i < 7; i++) {
        assert(pearl_contains(original, &original_values[i]) == true);
        assert(pearl_contains(roundtrip, &original_values[i]) == true);
    }
    
    pearl_destroy(original);
    pearl_destroy(roundtrip);
    samrena_destroy(arena1);
    samrena_destroy(arena2);
    printf("✓ Pearl roundtrip test passed\n");
}

static void test_pearl_phase3_error_conditions(void) {
    printf("Testing Pearl phase 3 error conditions...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    assert(pearl_copy(NULL, arena) == NULL);
    assert(pearl_copy(pearl, NULL) == NULL);
    
    assert(pearl_to_array(NULL, NULL, 0) == 0);
    assert(pearl_to_array(pearl, NULL, 10) == 0);
    
    assert(pearl_from_array(NULL, 10, sizeof(int), arena) == NULL);
    assert(pearl_from_array("test", 10, 0, arena) == NULL);
    assert(pearl_from_array("test", 10, sizeof(int), NULL) == NULL);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl phase 3 error conditions test passed\n");
}

int main(void) {
    printf("Running Pearl Phase 3 Tests...\n\n");
    
    test_pearl_copy();
    test_pearl_copy_empty();
    test_pearl_to_array();
    test_pearl_to_array_empty();
    test_pearl_from_array();
    test_pearl_from_array_empty();
    test_pearl_roundtrip();
    test_pearl_phase3_error_conditions();
    
    printf("\n✓ All Pearl Phase 3 tests passed!\n");
    return 0;
}
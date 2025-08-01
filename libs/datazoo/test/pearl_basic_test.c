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

static void test_pearl_creation(void) {
    printf("Testing Pearl creation...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    assert(pearl_size(pearl) == 0);
    assert(pearl_is_empty(pearl) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl creation test passed\n");
}

static void test_pearl_creation_with_hash(void) {
    printf("Testing Pearl creation with hash function...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_with_hash(sizeof(int), 16, arena, PEARL_HASH_FNV1A);
    assert(pearl != NULL);
    assert(pearl_size(pearl) == 0);
    assert(pearl_is_empty(pearl) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl creation with hash function test passed\n");
}

static void test_pearl_basic_operations(void) {
    printf("Testing Pearl basic operations...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        bool result = pearl_add(pearl, &values[i]);
        assert(result == true);
        assert(pearl_size(pearl) == (size_t)(i + 1));
        assert(pearl_is_empty(pearl) == false);
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    for (int i = 0; i < num_values; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    int non_existent = 999;
    assert(pearl_contains(pearl, &non_existent) == false);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl basic operations test passed\n");
}

static void test_pearl_duplicate_handling(void) {
    printf("Testing Pearl duplicate handling...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int value = 42;
    
    bool result1 = pearl_add(pearl, &value);
    assert(result1 == true);
    assert(pearl_size(pearl) == 1);
    
    bool result2 = pearl_add(pearl, &value);
    assert(result2 == false);
    assert(pearl_size(pearl) == 1);
    assert(pearl_get_last_error(pearl) == PEARL_ERROR_ELEMENT_EXISTS);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl duplicate handling test passed\n");
}

static void test_pearl_removal(void) {
    printf("Testing Pearl removal...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    int remove_value = 30;
    bool result = pearl_remove(pearl, &remove_value);
    assert(result == true);
    assert(pearl_size(pearl) == (size_t)(num_values - 1));
    assert(pearl_contains(pearl, &remove_value) == false);
    
    int non_existent = 999;
    result = pearl_remove(pearl, &non_existent);
    assert(result == false);
    assert(pearl_get_last_error(pearl) == PEARL_ERROR_ELEMENT_NOT_FOUND);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl removal test passed\n");
}

static void test_pearl_clear(void) {
    printf("Testing Pearl clear...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    assert(pearl_size(pearl) == (size_t)num_values);
    assert(pearl_is_empty(pearl) == false);
    
    pearl_clear(pearl);
    
    assert(pearl_size(pearl) == 0);
    assert(pearl_is_empty(pearl) == true);
    
    for (int i = 0; i < num_values; i++) {
        assert(pearl_contains(pearl, &values[i]) == false);
    }
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl clear test passed\n");
}

static void test_pearl_statistics(void) {
    printf("Testing Pearl statistics...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 4, arena);
    assert(pearl != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    PearlStats stats = pearl_get_stats(pearl);
    assert(stats.total_operations == (size_t)num_values);
    
    pearl_reset_stats(pearl);
    stats = pearl_get_stats(pearl);
    assert(stats.total_operations == 0);
    assert(stats.total_collisions == 0);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl statistics test passed\n");
}

static void test_pearl_null_parameters(void) {
    printf("Testing Pearl null parameter handling...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 16, NULL);
    assert(pearl == NULL);
    
    pearl = pearl_create(sizeof(int), 16, arena);
    assert(pearl != NULL);
    
    int value = 42;
    
    assert(pearl_add(NULL, &value) == false);
    assert(pearl_add(pearl, NULL) == false);
    assert(pearl_contains(NULL, &value) == false);
    assert(pearl_contains(pearl, NULL) == false);
    assert(pearl_remove(NULL, &value) == false);
    assert(pearl_remove(pearl, NULL) == false);
    assert(pearl_size(NULL) == 0);
    assert(pearl_is_empty(NULL) == true);
    
    pearl_clear(NULL);
    pearl_destroy(NULL);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Pearl null parameter handling test passed\n");
}

int main(void) {
    printf("=== Pearl Basic Tests ===\n");
    
    test_pearl_creation();
    test_pearl_creation_with_hash();
    test_pearl_basic_operations();
    test_pearl_duplicate_handling();
    test_pearl_removal();
    test_pearl_clear();
    test_pearl_statistics();
    test_pearl_null_parameters();
    
    printf("\n✅ All Pearl basic tests passed!\n");
    return 0;
}
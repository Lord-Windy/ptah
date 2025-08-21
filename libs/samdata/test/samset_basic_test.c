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

#include <samdata/samset.h>
#include <samrena.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_samset_creation(void) {
    printf("Testing SamSet creation...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 16, arena);
    assert(samset != NULL);
    assert(samset_size(samset) == 0);
    assert(samset_is_empty(samset) == true);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet creation test passed\n");
}

static void test_samset_creation_with_hash(void) {
    printf("Testing SamSet creation with hash function...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create_with_hash(sizeof(int), 16, arena, SAMSET_HASH_FNV1A);
    assert(samset != NULL);
    assert(samset_size(samset) == 0);
    assert(samset_is_empty(samset) == true);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet creation with hash function test passed\n");
}

static void test_samset_basic_operations(void) {
    printf("Testing SamSet basic operations...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 16, arena);
    assert(samset != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        bool result = samset_add(samset, &values[i]);
        assert(result == true);
        assert(samset_size(samset) == (size_t)(i + 1));
        assert(samset_is_empty(samset) == false);
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    for (int i = 0; i < num_values; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    int non_existent = 999;
    assert(samset_contains(samset, &non_existent) == false);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet basic operations test passed\n");
}

static void test_samset_duplicate_handling(void) {
    printf("Testing SamSet duplicate handling...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 16, arena);
    assert(samset != NULL);
    
    int value = 42;
    
    bool result1 = samset_add(samset, &value);
    assert(result1 == true);
    assert(samset_size(samset) == 1);
    
    bool result2 = samset_add(samset, &value);
    assert(result2 == false);
    assert(samset_size(samset) == 1);
    assert(samset_get_last_error(samset) == SAMSET_ERROR_ELEMENT_EXISTS);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet duplicate handling test passed\n");
}

static void test_samset_removal(void) {
    printf("Testing SamSet removal...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 16, arena);
    assert(samset != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        samset_add(samset, &values[i]);
    }
    
    int remove_value = 30;
    bool result = samset_remove(samset, &remove_value);
    assert(result == true);
    assert(samset_size(samset) == (size_t)(num_values - 1));
    assert(samset_contains(samset, &remove_value) == false);
    
    int non_existent = 999;
    result = samset_remove(samset, &non_existent);
    assert(result == false);
    assert(samset_get_last_error(samset) == SAMSET_ERROR_ELEMENT_NOT_FOUND);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet removal test passed\n");
}

static void test_samset_clear(void) {
    printf("Testing SamSet clear...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 16, arena);
    assert(samset != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        samset_add(samset, &values[i]);
    }
    
    assert(samset_size(samset) == (size_t)num_values);
    assert(samset_is_empty(samset) == false);
    
    samset_clear(samset);
    
    assert(samset_size(samset) == 0);
    assert(samset_is_empty(samset) == true);
    
    for (int i = 0; i < num_values; i++) {
        assert(samset_contains(samset, &values[i]) == false);
    }
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet clear test passed\n");
}

static void test_samset_statistics(void) {
    printf("Testing SamSet statistics...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 4, arena);
    assert(samset != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        samset_add(samset, &values[i]);
    }
    
    SamSetStats stats = samset_get_stats(samset);
    assert(stats.total_operations == (size_t)num_values);
    
    samset_reset_stats(samset);
    stats = samset_get_stats(samset);
    assert(stats.total_operations == 0);
    assert(stats.total_collisions == 0);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet statistics test passed\n");
}

static void test_samset_null_parameters(void) {
    printf("Testing SamSet null parameter handling...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 16, NULL);
    assert(samset == NULL);
    
    samset = samset_create(sizeof(int), 16, arena);
    assert(samset != NULL);
    
    int value = 42;
    
    assert(samset_add(NULL, &value) == false);
    assert(samset_add(samset, NULL) == false);
    assert(samset_contains(NULL, &value) == false);
    assert(samset_contains(samset, NULL) == false);
    assert(samset_remove(NULL, &value) == false);
    assert(samset_remove(samset, NULL) == false);
    assert(samset_size(NULL) == 0);
    assert(samset_is_empty(NULL) == true);
    
    samset_clear(NULL);
    samset_destroy(NULL);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ SamSet null parameter handling test passed\n");
}

int main(void) {
    printf("=== SamSet Basic Tests ===\n");
    
    test_samset_creation();
    test_samset_creation_with_hash();
    test_samset_basic_operations();
    test_samset_duplicate_handling();
    test_samset_removal();
    test_samset_clear();
    test_samset_statistics();
    test_samset_null_parameters();
    
    printf("\n✅ All SamSet basic tests passed!\n");
    return 0;
}
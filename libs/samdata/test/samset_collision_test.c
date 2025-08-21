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

static uint32_t collision_hash(const void *element, size_t size) {
    (void)element;
    (void)size;
    return 42;
}

static void test_samset_hash_collisions(void) {
    printf("Testing hash collisions with custom hash function...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create_custom(sizeof(int), 16, arena, collision_hash, NULL);
    assert(samset != NULL);
    
    int values[] = {10, 20, 30, 40, 50, 60, 70, 80};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        bool result = samset_add(samset, &values[i]);
        assert(result == true);
        assert(samset_size(samset) == (size_t)(i + 1));
    }
    
    for (int i = 0; i < num_values; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    SamSetStats stats = samset_get_stats(samset);
    assert(stats.total_collisions > 0);
    assert(stats.max_chain_length > 1);
    
    for (int i = 0; i < num_values; i++) {
        bool result = samset_remove(samset, &values[i]);
        assert(result == true);
        assert(samset_contains(samset, &values[i]) == false);
        assert(samset_size(samset) == (size_t)(num_values - i - 1));
    }
    
    assert(samset_is_empty(samset));
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Hash collision test passed\n");
}

static uint32_t modulo_hash(const void *element, size_t size) {
    (void)size;
    int value = *(const int*)element;
    return (uint32_t)(value % 4);
}

static void test_samset_collision_chain_management(void) {
    printf("Testing collision chain management...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create_custom(sizeof(int), 8, arena, modulo_hash, NULL);
    assert(samset != NULL);
    
    int values[] = {4, 8, 12, 16, 20, 24, 28, 32};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        bool result = samset_add(samset, &values[i]);
        assert(result == true);
    }
    
    for (int i = 0; i < num_values; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    bool result = samset_remove(samset, &values[3]);
    assert(result == true);
    assert(samset_contains(samset, &values[3]) == false);
    
    for (int i = 0; i < num_values; i++) {
        if (i != 3) {
            assert(samset_contains(samset, &values[i]) == true);
        }
    }
    
    result = samset_add(samset, &values[3]);
    assert(result == true);
    assert(samset_contains(samset, &values[3]) == true);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Collision chain management test passed\n");
}

static void test_samset_different_hash_functions(void) {
    printf("Testing different hash functions...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSetHashFunction functions[] = {SAMSET_HASH_DJB2, SAMSET_HASH_FNV1A, SAMSET_HASH_MURMUR3};
    int num_functions = sizeof(functions) / sizeof(functions[0]);
    
    int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int f = 0; f < num_functions; f++) {
        SamSet *samset = samset_create_with_hash(sizeof(int), 8, arena, functions[f]);
        assert(samset != NULL);
        
        for (int i = 0; i < num_values; i++) {
            bool result = samset_add(samset, &values[i]);
            assert(result == true);
        }
        
        for (int i = 0; i < num_values; i++) {
            assert(samset_contains(samset, &values[i]) == true);
        }
        
        assert(samset_size(samset) == (size_t)num_values);
        
        samset_destroy(samset);
    }
    
    samrena_destroy(arena);
    printf("✓ Different hash functions test passed\n");
}

static void test_samset_collision_statistics(void) {
    printf("Testing collision statistics...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create_custom(sizeof(int), 16, arena, collision_hash, NULL);
    assert(samset != NULL);
    
    int values[] = {1, 2, 3, 4, 5};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        samset_add(samset, &values[i]);
    }
    
    SamSetStats stats = samset_get_stats(samset);
    assert(stats.total_collisions == (size_t)(num_values - 1));
    assert(stats.max_chain_length >= 1);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Collision statistics test passed\n");
}

static void test_samset_collision_removal_edge_cases(void) {
    printf("Testing collision removal edge cases...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create_custom(sizeof(int), 4, arena, collision_hash, NULL);
    assert(samset != NULL);
    
    int values[] = {1, 2, 3, 4, 5};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        samset_add(samset, &values[i]);
    }
    
    bool result = samset_remove(samset, &values[0]);
    assert(result == true);
    assert(samset_contains(samset, &values[0]) == false);
    
    for (int i = 1; i < num_values; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    result = samset_remove(samset, &values[num_values - 1]);
    assert(result == true);
    assert(samset_contains(samset, &values[num_values - 1]) == false);
    
    for (int i = 1; i < num_values - 1; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    result = samset_remove(samset, &values[2]);
    assert(result == true);
    assert(samset_contains(samset, &values[2]) == false);
    
    assert(samset_contains(samset, &values[1]) == true);
    assert(samset_contains(samset, &values[3]) == true);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Collision removal edge cases test passed\n");
}

int main(void) {
    printf("=== SamSet Collision Tests ===\n");
    
    test_samset_hash_collisions();
    test_samset_collision_chain_management();
    test_samset_different_hash_functions();
    test_samset_collision_statistics();
    test_samset_collision_removal_edge_cases();
    
    printf("\n✅ All SamSet collision tests passed!\n");
    return 0;
}
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

static uint32_t collision_hash(const void *element, size_t size) {
    (void)element;
    (void)size;
    return 42;
}

static void test_pearl_hash_collisions(void) {
    printf("Testing hash collisions with custom hash function...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_custom(sizeof(int), 16, arena, collision_hash, NULL);
    assert(pearl != NULL);
    
    int values[] = {10, 20, 30, 40, 50, 60, 70, 80};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        bool result = pearl_add(pearl, &values[i]);
        assert(result == true);
        assert(pearl_size(pearl) == (size_t)(i + 1));
    }
    
    for (int i = 0; i < num_values; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    PearlStats stats = pearl_get_stats(pearl);
    assert(stats.total_collisions > 0);
    assert(stats.max_chain_length > 1);
    
    for (int i = 0; i < num_values; i++) {
        bool result = pearl_remove(pearl, &values[i]);
        assert(result == true);
        assert(pearl_contains(pearl, &values[i]) == false);
        assert(pearl_size(pearl) == (size_t)(num_values - i - 1));
    }
    
    assert(pearl_is_empty(pearl));
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Hash collision test passed\n");
}

static uint32_t modulo_hash(const void *element, size_t size) {
    (void)size;
    int value = *(const int*)element;
    return (uint32_t)(value % 4);
}

static void test_pearl_collision_chain_management(void) {
    printf("Testing collision chain management...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_custom(sizeof(int), 8, arena, modulo_hash, NULL);
    assert(pearl != NULL);
    
    int values[] = {4, 8, 12, 16, 20, 24, 28, 32};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        bool result = pearl_add(pearl, &values[i]);
        assert(result == true);
    }
    
    for (int i = 0; i < num_values; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    bool result = pearl_remove(pearl, &values[3]);
    assert(result == true);
    assert(pearl_contains(pearl, &values[3]) == false);
    
    for (int i = 0; i < num_values; i++) {
        if (i != 3) {
            assert(pearl_contains(pearl, &values[i]) == true);
        }
    }
    
    result = pearl_add(pearl, &values[3]);
    assert(result == true);
    assert(pearl_contains(pearl, &values[3]) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Collision chain management test passed\n");
}

static void test_pearl_different_hash_functions(void) {
    printf("Testing different hash functions...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    PearlHashFunction functions[] = {PEARL_HASH_DJB2, PEARL_HASH_FNV1A, PEARL_HASH_MURMUR3};
    int num_functions = sizeof(functions) / sizeof(functions[0]);
    
    int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int f = 0; f < num_functions; f++) {
        Pearl *pearl = pearl_create_with_hash(sizeof(int), 8, arena, functions[f]);
        assert(pearl != NULL);
        
        for (int i = 0; i < num_values; i++) {
            bool result = pearl_add(pearl, &values[i]);
            assert(result == true);
        }
        
        for (int i = 0; i < num_values; i++) {
            assert(pearl_contains(pearl, &values[i]) == true);
        }
        
        assert(pearl_size(pearl) == (size_t)num_values);
        
        pearl_destroy(pearl);
    }
    
    samrena_destroy(arena);
    printf("✓ Different hash functions test passed\n");
}

static void test_pearl_collision_statistics(void) {
    printf("Testing collision statistics...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_custom(sizeof(int), 16, arena, collision_hash, NULL);
    assert(pearl != NULL);
    
    int values[] = {1, 2, 3, 4, 5};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    PearlStats stats = pearl_get_stats(pearl);
    assert(stats.total_collisions == (size_t)(num_values - 1));
    assert(stats.max_chain_length >= 1);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Collision statistics test passed\n");
}

static void test_pearl_collision_removal_edge_cases(void) {
    printf("Testing collision removal edge cases...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_custom(sizeof(int), 4, arena, collision_hash, NULL);
    assert(pearl != NULL);
    
    int values[] = {1, 2, 3, 4, 5};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    bool result = pearl_remove(pearl, &values[0]);
    assert(result == true);
    assert(pearl_contains(pearl, &values[0]) == false);
    
    for (int i = 1; i < num_values; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    result = pearl_remove(pearl, &values[num_values - 1]);
    assert(result == true);
    assert(pearl_contains(pearl, &values[num_values - 1]) == false);
    
    for (int i = 1; i < num_values - 1; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    result = pearl_remove(pearl, &values[2]);
    assert(result == true);
    assert(pearl_contains(pearl, &values[2]) == false);
    
    assert(pearl_contains(pearl, &values[1]) == true);
    assert(pearl_contains(pearl, &values[3]) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Collision removal edge cases test passed\n");
}

int main(void) {
    printf("=== Pearl Collision Tests ===\n");
    
    test_pearl_hash_collisions();
    test_pearl_collision_chain_management();
    test_pearl_different_hash_functions();
    test_pearl_collision_statistics();
    test_pearl_collision_removal_edge_cases();
    
    printf("\n✅ All Pearl collision tests passed!\n");
    return 0;
}
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

static void test_samset_automatic_resize(void) {
    printf("Testing automatic resize when load factor exceeded...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 4, arena);
    assert(samset != NULL);
    
    int values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = i + 1;
    }
    
    SamSetStats stats_before = samset_get_stats(samset);
    assert(stats_before.resize_count == 0);
    
    for (int i = 0; i < 20; i++) {
        bool result = samset_add(samset, &values[i]);
        assert(result == true);
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    SamSetStats stats_after = samset_get_stats(samset);
    assert(stats_after.resize_count > 0);
    
    for (int i = 0; i < 20; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    assert(samset_size(samset) == 20);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Automatic resize test passed\n");
}

static void test_samset_resize_preserves_elements(void) {
    printf("Testing that resize preserves all elements...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 4, arena);
    assert(samset != NULL);
    
    int values[50];
    for (int i = 0; i < 50; i++) {
        values[i] = i * 2;
    }
    
    for (int i = 0; i < 50; i++) {
        samset_add(samset, &values[i]);
    }
    
    for (int i = 0; i < 50; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    assert(samset_size(samset) == 50);
    
    int non_existent = 999;
    assert(samset_contains(samset, &non_existent) == false);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Resize preserves elements test passed\n");
}

static uint32_t modulo_hash_func(const void *element, size_t size) {
    (void)size;
    int value = *(const int*)element;
    return (uint32_t)(value % 3);
}

static void test_samset_resize_with_collisions(void) {
    printf("Testing resize with hash collisions...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create_custom(sizeof(int), 4, arena, modulo_hash_func, NULL);
    assert(samset != NULL);
    
    int values[30];
    for (int i = 0; i < 30; i++) {
        values[i] = i;
    }
    
    for (int i = 0; i < 30; i++) {
        samset_add(samset, &values[i]);
    }
    
    SamSetStats stats = samset_get_stats(samset);
    assert(stats.resize_count > 0);
    assert(stats.total_collisions > 0);
    
    for (int i = 0; i < 30; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    assert(samset_size(samset) == 30);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Resize with collisions test passed\n");
}

static void test_samset_multiple_resizes(void) {
    printf("Testing multiple consecutive resizes...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 2, arena);
    assert(samset != NULL);
    
    const int num_elements = 100;
    int values[num_elements];
    for (int i = 0; i < num_elements; i++) {
        values[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < num_elements; i++) {
        samset_add(samset, &values[i]);
        
        if (i % 10 == 9) {
            for (int j = 0; j <= i; j++) {
                assert(samset_contains(samset, &values[j]) == true);
            }
        }
    }
    
    SamSetStats stats = samset_get_stats(samset);
    assert(stats.resize_count >= 3);
    
    for (int i = 0; i < num_elements; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    assert(samset_size(samset) == (size_t)num_elements);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Multiple resizes test passed\n");
}

static void test_samset_load_factor_threshold(void) {
    printf("Testing load factor threshold behavior...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 8, arena);
    assert(samset != NULL);
    
    SamSetStats initial_stats = samset_get_stats(samset);
    assert(initial_stats.resize_count == 0);
    
    int values[6];
    for (int i = 0; i < 6; i++) {
        values[i] = i + 100;
        samset_add(samset, &values[i]);
    }
    
    SamSetStats stats_at_6 = samset_get_stats(samset);
    
    int value_7 = 107;
    samset_add(samset, &value_7);
    
    SamSetStats stats_at_7 = samset_get_stats(samset);
    assert(stats_at_7.resize_count >= stats_at_6.resize_count);
    
    for (int i = 0; i < 6; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    assert(samset_contains(samset, &value_7) == true);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Load factor threshold test passed\n");
}

static void test_samset_resize_after_removals(void) {
    printf("Testing behavior after removals and subsequent additions...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    SamSet *samset = samset_create(sizeof(int), 4, arena);
    assert(samset != NULL);
    
    int values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = i + 200;
        samset_add(samset, &values[i]);
    }
    
    SamSetStats stats_after_adds = samset_get_stats(samset);
    size_t resize_count_after_adds = stats_after_adds.resize_count;
    
    for (int i = 0; i < 10; i++) {
        samset_remove(samset, &values[i]);
    }
    
    for (int i = 10; i < 20; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    
    int new_values[15];
    for (int i = 0; i < 15; i++) {
        new_values[i] = i + 300;
        samset_add(samset, &new_values[i]);
    }
    
    SamSetStats final_stats = samset_get_stats(samset);
    assert(final_stats.resize_count >= resize_count_after_adds);
    
    for (int i = 10; i < 20; i++) {
        assert(samset_contains(samset, &values[i]) == true);
    }
    for (int i = 0; i < 15; i++) {
        assert(samset_contains(samset, &new_values[i]) == true);
    }
    
    assert(samset_size(samset) == 25);
    
    samset_destroy(samset);
    samrena_destroy(arena);
    printf("✓ Resize after removals test passed\n");
}

int main(void) {
    printf("=== SamSet Resize Tests ===\n");
    
    test_samset_automatic_resize();
    test_samset_resize_preserves_elements();
    test_samset_resize_with_collisions();
    test_samset_multiple_resizes();
    test_samset_load_factor_threshold();
    test_samset_resize_after_removals();
    
    printf("\n✅ All SamSet resize tests passed!\n");
    return 0;
}
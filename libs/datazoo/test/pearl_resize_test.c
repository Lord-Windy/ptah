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

static void test_automatic_resize(void) {
    printf("Testing automatic resize when load factor exceeded...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 4, arena);
    assert(pearl != NULL);
    
    int values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = i + 1;
    }
    
    PearlStats stats_before = pearl_get_stats(pearl);
    assert(stats_before.resize_count == 0);
    
    for (int i = 0; i < 20; i++) {
        bool result = pearl_add(pearl, &values[i]);
        assert(result == true);
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    PearlStats stats_after = pearl_get_stats(pearl);
    assert(stats_after.resize_count > 0);
    
    for (int i = 0; i < 20; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    assert(pearl_size(pearl) == 20);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Automatic resize test passed\n");
}

static void test_resize_preserves_elements(void) {
    printf("Testing that resize preserves all elements...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 4, arena);
    assert(pearl != NULL);
    
    int values[50];
    for (int i = 0; i < 50; i++) {
        values[i] = i * 2;
    }
    
    for (int i = 0; i < 50; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    for (int i = 0; i < 50; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    assert(pearl_size(pearl) == 50);
    
    int non_existent = 999;
    assert(pearl_contains(pearl, &non_existent) == false);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Resize preserves elements test passed\n");
}

static uint32_t modulo_hash_func(const void *element, size_t size) {
    (void)size;
    int value = *(const int*)element;
    return (uint32_t)(value % 3);
}

static void test_resize_with_collisions(void) {
    printf("Testing resize with hash collisions...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create_custom(sizeof(int), 4, arena, modulo_hash_func, NULL);
    assert(pearl != NULL);
    
    int values[30];
    for (int i = 0; i < 30; i++) {
        values[i] = i;
    }
    
    for (int i = 0; i < 30; i++) {
        pearl_add(pearl, &values[i]);
    }
    
    PearlStats stats = pearl_get_stats(pearl);
    assert(stats.resize_count > 0);
    assert(stats.total_collisions > 0);
    
    for (int i = 0; i < 30; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    assert(pearl_size(pearl) == 30);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Resize with collisions test passed\n");
}

static void test_multiple_resizes(void) {
    printf("Testing multiple consecutive resizes...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 2, arena);
    assert(pearl != NULL);
    
    const int num_elements = 100;
    int values[num_elements];
    for (int i = 0; i < num_elements; i++) {
        values[i] = i * 3 + 1;
    }
    
    for (int i = 0; i < num_elements; i++) {
        pearl_add(pearl, &values[i]);
        
        if (i % 10 == 9) {
            for (int j = 0; j <= i; j++) {
                assert(pearl_contains(pearl, &values[j]) == true);
            }
        }
    }
    
    PearlStats stats = pearl_get_stats(pearl);
    assert(stats.resize_count >= 3);
    
    for (int i = 0; i < num_elements; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    assert(pearl_size(pearl) == (size_t)num_elements);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Multiple resizes test passed\n");
}

static void test_load_factor_threshold(void) {
    printf("Testing load factor threshold behavior...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 8, arena);
    assert(pearl != NULL);
    
    PearlStats initial_stats = pearl_get_stats(pearl);
    assert(initial_stats.resize_count == 0);
    
    int values[6];
    for (int i = 0; i < 6; i++) {
        values[i] = i + 100;
        pearl_add(pearl, &values[i]);
    }
    
    PearlStats stats_at_6 = pearl_get_stats(pearl);
    
    int value_7 = 107;
    pearl_add(pearl, &value_7);
    
    PearlStats stats_at_7 = pearl_get_stats(pearl);
    assert(stats_at_7.resize_count >= stats_at_6.resize_count);
    
    for (int i = 0; i < 6; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    assert(pearl_contains(pearl, &value_7) == true);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Load factor threshold test passed\n");
}

static void test_resize_after_removals(void) {
    printf("Testing behavior after removals and subsequent additions...\n");
    
    SamrenaConfig config = samrena_default_config();
    Samrena *arena = samrena_create(&config);
    assert(arena != NULL);
    
    Pearl *pearl = pearl_create(sizeof(int), 4, arena);
    assert(pearl != NULL);
    
    int values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = i + 200;
        pearl_add(pearl, &values[i]);
    }
    
    PearlStats stats_after_adds = pearl_get_stats(pearl);
    size_t resize_count_after_adds = stats_after_adds.resize_count;
    
    for (int i = 0; i < 10; i++) {
        pearl_remove(pearl, &values[i]);
    }
    
    for (int i = 10; i < 20; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    
    int new_values[15];
    for (int i = 0; i < 15; i++) {
        new_values[i] = i + 300;
        pearl_add(pearl, &new_values[i]);
    }
    
    PearlStats final_stats = pearl_get_stats(pearl);
    assert(final_stats.resize_count >= resize_count_after_adds);
    
    for (int i = 10; i < 20; i++) {
        assert(pearl_contains(pearl, &values[i]) == true);
    }
    for (int i = 0; i < 15; i++) {
        assert(pearl_contains(pearl, &new_values[i]) == true);
    }
    
    assert(pearl_size(pearl) == 25);
    
    pearl_destroy(pearl);
    samrena_destroy(arena);
    printf("✓ Resize after removals test passed\n");
}

int main(void) {
    printf("=== Pearl Resize Tests ===\n");
    
    test_automatic_resize();
    test_resize_preserves_elements();
    test_resize_with_collisions();
    test_multiple_resizes();
    test_load_factor_threshold();
    test_resize_after_removals();
    
    printf("\n✅ All Pearl resize tests passed!\n");
    return 0;
}
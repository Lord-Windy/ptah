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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "samrena.h"
#include "samvector.h"

void test_vector_reserve() {
    printf("Testing samrena_vector_reserve...\n");
    
    Samrena* arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 4);
    assert(vec != NULL);
    assert(samrena_vector_capacity(vec) == 4);
    
    // Test reserving more capacity
    SamrenaVectorError err = samrena_vector_reserve(vec, 20);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) >= 20);
    assert(samrena_vector_size(vec) == 0);
    
    // Test reserving less than current capacity (should be no-op)
    size_t current_cap = samrena_vector_capacity(vec);
    err = samrena_vector_reserve(vec, 10);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) == current_cap);
    
    // Test with data
    int values[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(arena, vec, &values[i]);
    }
    assert(samrena_vector_size(vec) == 5);
    
    err = samrena_vector_reserve(vec, 50);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) >= 50);
    assert(samrena_vector_size(vec) == 5);
    
    // Verify data integrity
    for (int i = 0; i < 5; i++) {
        int* val = (int*)samrena_vector_at(vec, i);
        assert(*val == values[i]);
    }
    
    samrena_destroy(arena);
    printf("  ✓ Reserve tests passed\n");
}

void test_vector_shrink_to_fit() {
    printf("Testing samrena_vector_shrink_to_fit...\n");
    
    Samrena* arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 20);
    assert(vec != NULL);
    assert(samrena_vector_capacity(vec) == 20);
    
    // Add some elements
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(arena, vec, &i);
    }
    assert(samrena_vector_size(vec) == 5);
    assert(samrena_vector_capacity(vec) == 20);
    
    // Shrink to fit
    SamrenaVectorError err = samrena_vector_shrink_to_fit(vec);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) == 5);
    assert(samrena_vector_size(vec) == 5);
    
    // Verify data integrity
    for (int i = 0; i < 5; i++) {
        int* val = (int*)samrena_vector_at(vec, i);
        assert(*val == i);
    }
    
    // Test empty vector
    samrena_vector_clear(vec);
    err = samrena_vector_shrink_to_fit(vec);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) == 0);
    assert(samrena_vector_size(vec) == 0);
    
    samrena_destroy(arena);
    printf("  ✓ Shrink to fit tests passed\n");
}

void test_vector_clear_and_truncate() {
    printf("Testing clear and truncate...\n");
    
    Samrena* arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 10);
    assert(vec != NULL);
    
    // Add elements
    for (int i = 0; i < 8; i++) {
        samrena_vector_push(arena, vec, &i);
    }
    assert(samrena_vector_size(vec) == 8);
    size_t cap = samrena_vector_capacity(vec);
    
    // Test clear
    samrena_vector_clear(vec);
    assert(samrena_vector_size(vec) == 0);
    assert(samrena_vector_capacity(vec) == cap); // Capacity unchanged
    assert(samrena_vector_is_empty(vec));
    
    // Add elements again
    for (int i = 10; i < 15; i++) {
        samrena_vector_push(arena, vec, &i);
    }
    assert(samrena_vector_size(vec) == 5);
    
    // Test truncate
    SamrenaVectorError err = samrena_vector_truncate(vec, 3);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_size(vec) == 3);
    
    // Verify remaining data
    for (int i = 0; i < 3; i++) {
        int* val = (int*)samrena_vector_at(vec, i);
        assert(*val == 10 + i);
    }
    
    // Test invalid truncate
    err = samrena_vector_truncate(vec, 10);
    assert(err == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_size(vec) == 3); // Size unchanged
    
    samrena_destroy(arena);
    printf("  ✓ Clear and truncate tests passed\n");
}

void test_vector_query_functions() {
    printf("Testing query functions...\n");
    
    Samrena* arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(double), 10);
    assert(vec != NULL);
    
    // Test empty vector
    assert(samrena_vector_size(vec) == 0);
    assert(samrena_vector_capacity(vec) == 10);
    assert(samrena_vector_is_empty(vec));
    assert(!samrena_vector_is_full(vec));
    assert(samrena_vector_available(vec) == 10);
    
    // Add elements
    double values[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(arena, vec, &values[i]);
    }
    
    assert(samrena_vector_size(vec) == 5);
    assert(samrena_vector_capacity(vec) == 10);
    assert(!samrena_vector_is_empty(vec));
    assert(!samrena_vector_is_full(vec));
    assert(samrena_vector_available(vec) == 5);
    
    // Fill to capacity
    for (int i = 5; i < 10; i++) {
        double val = i * 1.1;
        samrena_vector_push(arena, vec, &val);
    }
    
    assert(samrena_vector_size(vec) == 10);
    assert(samrena_vector_capacity(vec) == 10);
    assert(!samrena_vector_is_empty(vec));
    assert(samrena_vector_is_full(vec));
    assert(samrena_vector_available(vec) == 0);
    
    // Test with NULL
    assert(samrena_vector_size(NULL) == 0);
    assert(samrena_vector_capacity(NULL) == 0);
    assert(samrena_vector_is_empty(NULL));
    assert(!samrena_vector_is_full(NULL));
    assert(samrena_vector_available(NULL) == 0);
    
    samrena_destroy(arena);
    printf("  ✓ Query function tests passed\n");
}

void test_vector_growth_control() {
    printf("Testing growth control...\n");
    
    Samrena* arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 2);
    assert(vec != NULL);
    
    // Test default growth
    assert(vec->growth_factor == 1.5f);
    assert(vec->min_growth == 8);
    
    // Set custom growth factor
    samrena_vector_set_growth_factor(vec, 2.0f);
    assert(vec->growth_factor == 2.0f);
    
    // Set minimum growth
    samrena_vector_set_min_growth(vec, 16);
    assert(vec->min_growth == 16);
    
    // Test growth with custom settings
    int val = 1;
    samrena_vector_push(arena, vec, &val);
    samrena_vector_push(arena, vec, &val);
    assert(samrena_vector_size(vec) == 2);
    assert(samrena_vector_capacity(vec) == 2);
    
    // Next push should trigger growth
    samrena_vector_push(arena, vec, &val);
    // With min_growth=16, capacity should be at least 18 (2 + 16)
    assert(samrena_vector_capacity(vec) >= 18);
    
    // Test stats
    SamrenaVectorStats stats = samrena_vector_get_stats(vec);
    assert(stats.used_bytes == 3 * sizeof(int));
    assert(stats.allocated_bytes == samrena_vector_capacity(vec) * sizeof(int));
    assert(stats.wasted_bytes == stats.allocated_bytes - stats.used_bytes);
    assert(stats.utilization > 0.0f && stats.utilization <= 1.0f);
    
    samrena_destroy(arena);
    printf("  ✓ Growth control tests passed\n");
}

void test_vector_memory_ownership() {
    printf("Testing memory ownership...\n");
    
    // Test vector with owned arena
    SamrenaVector* owned_vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(owned_vec != NULL);
    assert(owned_vec->owns_arena == true);
    assert(owned_vec->arena != NULL);
    
    // Use the vector
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(owned_vec->arena, owned_vec, &i);
    }
    assert(samrena_vector_size(owned_vec) == 5);
    
    // Clean up - should destroy the arena too
    samrena_vector_destroy(owned_vec);
    
    // Test vector with external arena
    Samrena* external_arena = samrena_create_default();
    assert(external_arena != NULL);
    
    SamrenaVector* external_vec = samrena_vector_init_with_arena(external_arena, sizeof(int), 10);
    assert(external_vec != NULL);
    assert(external_vec->owns_arena == false);
    assert(external_vec->arena == external_arena);
    
    // Use the vector
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(external_arena, external_vec, &i);
    }
    assert(samrena_vector_size(external_vec) == 5);
    
    // Clean up - should not destroy the arena
    samrena_vector_destroy(external_vec);
    
    // Arena should still be valid
    void* test_alloc = samrena_push(external_arena, 100);
    assert(test_alloc != NULL);
    
    samrena_destroy(external_arena);
    printf("  ✓ Memory ownership tests passed\n");
}

void test_vector_reset() {
    printf("Testing vector reset...\n");
    
    Samrena* arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 10);
    assert(vec != NULL);
    
    // Add elements
    for (int i = 0; i < 8; i++) {
        samrena_vector_push(arena, vec, &i);
    }
    assert(samrena_vector_size(vec) == 8);
    assert(samrena_vector_capacity(vec) == 10);
    
    // Reset with same capacity
    SamrenaVectorError err = samrena_vector_reset(vec, 10);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_size(vec) == 0);
    assert(samrena_vector_capacity(vec) == 10);
    
    // Add new elements
    for (int i = 100; i < 105; i++) {
        samrena_vector_push(arena, vec, &i);
    }
    assert(samrena_vector_size(vec) == 5);
    
    // Reset with different capacity
    err = samrena_vector_reset(vec, 20);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_size(vec) == 0);
    assert(samrena_vector_capacity(vec) == 20);
    
    samrena_destroy(arena);
    printf("  ✓ Reset tests passed\n");
}

void test_vector_set_capacity() {
    printf("Testing set capacity...\n");
    
    Samrena* arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 10);
    assert(vec != NULL);
    
    // Add elements
    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(arena, vec, &values[i]);
    }
    assert(samrena_vector_size(vec) == 5);
    assert(samrena_vector_capacity(vec) == 10);
    
    // Increase capacity
    SamrenaVectorError err = samrena_vector_set_capacity(vec, 20);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) == 20);
    assert(samrena_vector_size(vec) == 5);
    
    // Verify data preserved
    for (int i = 0; i < 5; i++) {
        int* val = (int*)samrena_vector_at(vec, i);
        assert(*val == values[i]);
    }
    
    // Decrease capacity (truncates)
    err = samrena_vector_set_capacity(vec, 3);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) == 3);
    assert(samrena_vector_size(vec) == 3);
    
    // Verify remaining data
    for (int i = 0; i < 3; i++) {
        int* val = (int*)samrena_vector_at(vec, i);
        assert(*val == values[i]);
    }
    
    // Set to zero
    err = samrena_vector_set_capacity(vec, 0);
    assert(err == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_capacity(vec) == 0);
    assert(samrena_vector_size(vec) == 0);
    assert(vec->data == NULL);
    
    samrena_destroy(arena);
    printf("  ✓ Set capacity tests passed\n");
}

int main() {
    printf("Running samvector capacity management tests...\n\n");
    
    test_vector_reserve();
    test_vector_shrink_to_fit();
    test_vector_clear_and_truncate();
    test_vector_query_functions();
    test_vector_growth_control();
    test_vector_memory_ownership();
    test_vector_reset();
    test_vector_set_capacity();
    
    printf("\nAll capacity management tests passed! ✨\n");
    return 0;
}
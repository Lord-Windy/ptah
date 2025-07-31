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

#include "samrena.h"
#include "samvector.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

void test_vector_large_dataset_operations() {
    printf("Testing vector with large datasets... ");
    
    const size_t large_count = 100000;
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 1000);
    assert(vec != NULL);
    
    // Test pushing large number of elements
    for (size_t i = 0; i < large_count; i++) {
        int value = (int)(i * 17 + 42); // Some pattern
        void *result = samrena_vector_push(vec, &value);
        assert(result != NULL);
        
        if (i % 10000 == 0) {
            assert(vec->size == i + 1);
        }
    }
    
    assert(vec->size == large_count);
    
    // Test random access across large dataset
    size_t test_indices[] = {0, 1, 100, 1000, 10000, 50000, 99999};
    for (size_t i = 0; i < sizeof(test_indices)/sizeof(test_indices[0]); i++) {
        size_t idx = test_indices[i];
        int expected = (int)(idx * 17 + 42);
        
        int retrieved;
        assert(samrena_vector_get(vec, idx, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == expected);
        
        int *ptr = (int*)samrena_vector_at(vec, idx);
        assert(ptr != NULL);
        assert(*ptr == expected);
    }
    
    // Test popping all elements
    for (size_t i = large_count; i > 0; i--) {
        int expected = (int)((i - 1) * 17 + 42);
        int *popped = (int*)samrena_vector_pop(vec);
        assert(popped != NULL);
        assert(*popped == expected);
        
        if (i % 10000 == 0) {
            assert(vec->size == i - 1);
        }
    }
    
    assert(samrena_vector_is_empty(vec) == true);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_memory_usage_patterns() {
    printf("Testing vector memory usage patterns... ");
    
    // Test with different element sizes and counts
    struct {
        size_t element_size;
        size_t count;
        const char *description;
    } test_cases[] = {
        {sizeof(char), 50000, "char"},
        {sizeof(int), 25000, "int"},
        {sizeof(double), 12500, "double"},
        {sizeof(void*), 20000, "pointer"},
        {64, 5000, "64-byte struct"},
        {256, 2000, "256-byte struct"},
        {1024, 500, "1KB struct"}
    };
    
    for (size_t tc = 0; tc < sizeof(test_cases)/sizeof(test_cases[0]); tc++) {
        SamrenaVector *vec = samrena_vector_init_owned(test_cases[tc].element_size, 100);
        assert(vec != NULL);
        
        // Fill with data
        char *dummy_data = malloc(test_cases[tc].element_size);
        memset(dummy_data, 0xAA, test_cases[tc].element_size);
        
        for (size_t i = 0; i < test_cases[tc].count; i++) {
            void *result = samrena_vector_push(vec, dummy_data);
            assert(result != NULL);
        }
        
        assert(vec->size == test_cases[tc].count);
        
        // Verify memory usage is reasonable
        size_t expected_min_bytes = test_cases[tc].count * test_cases[tc].element_size;
        size_t actual_bytes = vec->capacity * vec->element_size;
        
        // Should not waste more than 2x memory
        assert(actual_bytes <= expected_min_bytes * 2);
        
        // Should not be using significantly less than needed
        assert(actual_bytes >= expected_min_bytes);
        
        free(dummy_data);
        samrena_vector_destroy(vec);
    }
    
    printf("PASSED\n");
}

void test_vector_growth_efficiency() {
    printf("Testing vector growth efficiency... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 1);
    assert(vec != NULL);
    
    // Track growth events
    size_t growth_count = 0;
    size_t last_capacity = vec->capacity;
    
    // Push elements and track growth
    const size_t target_size = 10000;
    for (size_t i = 0; i < target_size; i++) {
        int value = (int)i;
        samrena_vector_push(vec, &value);
        
        if (vec->capacity > last_capacity) {
            growth_count++;
            
            // Growth should be reasonable (at least 50% increase)
            assert(vec->capacity >= (size_t)(last_capacity * 1.5));
            
            last_capacity = vec->capacity;
        }
    }
    
    // Should not grow too frequently (should be logarithmic)
    // With typical growth factors (1.5-2.0), we expect ~20-30 growths for 10K elements
    assert(growth_count < 50);
    assert(growth_count > 5);
    
    printf("Growth events: %zu for %zu elements\n", growth_count, target_size);
    
    // Verify all elements are still correct
    for (size_t i = 0; i < target_size; i += 1000) { // Sample every 1000th element
        int retrieved;
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == (int)i);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_capacity_utilization() {
    printf("Testing vector capacity utilization... ");
    
    // Test different usage patterns
    struct {
        size_t initial_capacity;
        size_t fill_count;
        const char *pattern;
    } patterns[] = {
        {10, 10, "exact_fill"},
        {100, 50, "half_fill"},
        {1000, 1500, "overflow"},
        {5, 200, "massive_overflow"}
    };
    
    for (size_t p = 0; p < sizeof(patterns)/sizeof(patterns[0]); p++) {
        SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), patterns[p].initial_capacity);
        assert(vec != NULL);
        
        // Fill according to pattern
        for (size_t i = 0; i < patterns[p].fill_count; i++) {
            int value = (int)(i + p * 10000); // Unique values per pattern
            samrena_vector_push(vec, &value);
        }
        
        assert(vec->size == patterns[p].fill_count);
        
        // Calculate utilization
        double utilization = (double)vec->size / (double)vec->capacity;
        
        // Should have reasonable utilization (at least 25% unless deliberately testing small fills)
        if (patterns[p].fill_count >= patterns[p].initial_capacity) {
            assert(utilization >= 0.25);
        }
        
        // Should not have extreme over-allocation (more than 4x needed)
        assert(utilization >= 0.25 || vec->capacity <= patterns[p].fill_count * 4);
        
        samrena_vector_destroy(vec);
    }
    
    printf("PASSED\n");
}

void test_vector_stress_push_pop_cycles() {
    printf("Testing vector stress with push/pop cycles... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 100);
    assert(vec != NULL);
    
    const size_t num_cycles = 1000;
    const size_t cycle_size = 50;
    
    for (size_t cycle = 0; cycle < num_cycles; cycle++) {
        // Push phase
        for (size_t i = 0; i < cycle_size; i++) {
            int value = (int)(cycle * cycle_size + i);
            void *result = samrena_vector_push(vec, &value);
            assert(result != NULL);
        }
        
        assert(vec->size == cycle_size);
        
        // Verify some elements
        if (cycle % 100 == 0) {
            for (size_t i = 0; i < cycle_size; i += 10) {
                int expected = (int)(cycle * cycle_size + i);
                int retrieved;
                assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
                assert(retrieved == expected);
            }
        }
        
        // Pop phase
        for (size_t i = 0; i < cycle_size; i++) {
            int expected = (int)(cycle * cycle_size + (cycle_size - 1 - i));
            int *popped = (int*)samrena_vector_pop(vec);
            assert(popped != NULL);
            assert(*popped == expected);
        }
        
        assert(vec->size == 0);
        assert(samrena_vector_is_empty(vec) == true);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_random_access_performance() {
    printf("Testing vector random access performance... ");
    
    const size_t vector_size = 10000;
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), vector_size);
    assert(vec != NULL);
    
    // Fill vector
    for (size_t i = 0; i < vector_size; i++) {
        int value = (int)(i * i % 7919); // Some pseudo-random pattern
        samrena_vector_push(vec, &value);
    }
    
    // Perform many random accesses
    const size_t num_accesses = 50000;
    for (size_t access = 0; access < num_accesses; access++) {
        size_t idx = access % vector_size; // Pseudo-random index
        
        int expected = (int)(idx * idx % 7919);
        
        // Test different access methods
        if (access % 3 == 0) {
            // Use samrena_vector_get
            int retrieved;
            assert(samrena_vector_get(vec, idx, &retrieved) == SAMRENA_VECTOR_SUCCESS);
            assert(retrieved == expected);
        } else if (access % 3 == 1) {
            // Use samrena_vector_at
            int *ptr = (int*)samrena_vector_at(vec, idx);
            assert(ptr != NULL);
            assert(*ptr == expected);
        } else {
            // Use macro access (when we know it's safe)
            if (idx < vec->size) {
                assert(SAMRENA_VECTOR_ELEM(vec, int, idx) == expected);
            }
        }
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_memory_locality() {
    printf("Testing vector memory locality... ");
    
    const size_t array_size = 10000;
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), array_size);
    assert(vec != NULL);
    
    // Fill vector
    for (size_t i = 0; i < array_size; i++) {
        int value = (int)i;
        samrena_vector_push(vec, &value);
    }
    
    // Test sequential access (should be cache-friendly)
    int sum = 0;
    for (size_t i = 0; i < array_size; i++) {
        int *ptr = (int*)samrena_vector_at(vec, i);
        assert(ptr != NULL);
        sum += *ptr;
    }
    
    // Verify sum (arithmetic series)
    int expected_sum = (int)(array_size * (array_size - 1) / 2);
    assert(sum == expected_sum);
    
    // Test that elements are stored contiguously
    int *first = (int*)samrena_vector_at(vec, 0);
    int *second = (int*)samrena_vector_at(vec, 1);
    assert(first != NULL && second != NULL);
    
    // Elements should be adjacent in memory
    ptrdiff_t diff = (char*)second - (char*)first;
    assert(diff == (ptrdiff_t)sizeof(int));
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_mixed_operations_stress() {
    printf("Testing vector mixed operations stress test... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 50);
    assert(vec != NULL);
    
    const size_t num_operations = 10000;
    size_t expected_size = 0;
    
    for (size_t op = 0; op < num_operations; op++) {
        size_t op_type = op % 7;
        
        switch (op_type) {
            case 0: case 1: case 2: // Push (more likely)
                {
                    int value = (int)op;
                    void *result = samrena_vector_push(vec, &value);
                    assert(result != NULL);
                    expected_size++;
                }
                break;
                
            case 3: // Pop (if not empty)
                if (expected_size > 0) {
                    int *popped = (int*)samrena_vector_pop(vec);
                    assert(popped != NULL);
                    expected_size--;
                }
                break;
                
            case 4: // Random access (if not empty)
                if (expected_size > 0) {
                    size_t idx = op % expected_size;
                    int retrieved;
                    assert(samrena_vector_get(vec, idx, &retrieved) == SAMRENA_VECTOR_SUCCESS);
                }
                break;
                
            case 5: // Clear occasionally
                if (op % 500 == 0 && expected_size > 100) {
                    samrena_vector_clear(vec);
                    expected_size = 0;
                }
                break;
                
            case 6: // Resize occasionally
                if (op % 300 == 0 && expected_size > 0) {
                    size_t new_capacity = vec->capacity + 100;
                    assert(samrena_vector_resize(vec, new_capacity) == SAMRENA_VECTOR_SUCCESS);
                }
                break;
        }
        
        // Verify size consistency
        assert(vec->size == expected_size);
        
        // Verify basic invariants
        assert(vec->capacity >= vec->size);
        if (expected_size == 0) {
            assert(samrena_vector_is_empty(vec) == true);
        } else {
            assert(samrena_vector_is_empty(vec) == false);
        }
        
        assert(samrena_vector_available(vec) == vec->capacity - vec->size);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

int main() {
    printf("Starting samrena vector performance tests...\n\n");

    test_vector_large_dataset_operations();
    test_vector_memory_usage_patterns();
    test_vector_growth_efficiency();
    test_vector_capacity_utilization();
    test_vector_stress_push_pop_cycles();
    test_vector_random_access_performance();
    test_vector_memory_locality();
    test_vector_mixed_operations_stress();

    printf("\nAll samrena vector performance tests passed!\n");
    return 0;
}
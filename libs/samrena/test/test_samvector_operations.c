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

void test_vector_push_basic() {
    printf("Testing samrena_vector_push basic functionality... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    
    // Push elements and verify return values
    for (int i = 0; i < 5; i++) {
        void *result = samrena_vector_push(vec, &values[i]);
        assert(result != NULL);
        assert(vec->size == (uint64_t)(i + 1));
        
        // Verify the returned pointer points to the correct element
        assert(*(int*)result == values[i]);
    }
    
    // Verify all elements are correct
    for (int i = 0; i < 5; i++) {
        int retrieved;
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == values[i]);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_push_growth() {
    printf("Testing samrena_vector_push with automatic growth... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 3);
    assert(vec != NULL);
    assert(vec->capacity == 3);
    
    int values[] = {1, 2, 3, 4, 5, 6, 7, 8};
    
    // Push 3 elements (should fit exactly)
    for (int i = 0; i < 3; i++) {
        void *result = samrena_vector_push(vec, &values[i]);
        assert(result != NULL);
        assert(vec->capacity == 3);
    }
    
    assert(samrena_vector_is_full(vec) == true);
    
    // Push 4th element (should trigger growth)
    void *result = samrena_vector_push(vec, &values[3]);
    assert(result != NULL);
    assert(vec->size == 4);
    assert(vec->capacity > 3);
    
    size_t new_capacity = vec->capacity;
    
    // Continue pushing to test growth behavior
    for (int i = 4; i < 8; i++) {
        result = samrena_vector_push(vec, &values[i]);
        assert(result != NULL);
        
        // Capacity should only grow when needed
        if (vec->size > new_capacity) {
            assert(vec->capacity > new_capacity);
            new_capacity = vec->capacity;
        }
    }
    
    assert(vec->size == 8);
    
    // Verify all elements are correct after growth
    for (int i = 0; i < 8; i++) {
        int retrieved;
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == values[i]);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_push_null_cases() {
    printf("Testing samrena_vector_push with NULL parameters... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    int value = 42;
    
    // Test NULL vector
    assert(samrena_vector_push(NULL, &value) == NULL);
    
    // Test NULL element
    assert(samrena_vector_push(vec, NULL) == NULL);
    
    // Vector should remain unchanged
    assert(vec->size == 0);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_push_different_types() {
    printf("Testing samrena_vector_push with different data types... ");
    
    // Test with char
    SamrenaVector *char_vec = samrena_vector_init_owned(sizeof(char), 5);
    char chars[] = {'A', 'B', 'C', 'D', 'E'};
    
    for (int i = 0; i < 5; i++) {
        char *result = (char*)samrena_vector_push(char_vec, &chars[i]);
        assert(result != NULL);
        assert(*result == chars[i]);
    }
    
    // Test with double
    SamrenaVector *double_vec = samrena_vector_init_owned(sizeof(double), 3);
    double doubles[] = {1.1, 2.2, 3.3};
    
    for (int i = 0; i < 3; i++) {
        double *result = (double*)samrena_vector_push(double_vec, &doubles[i]);
        assert(result != NULL);
        assert(*result == doubles[i]);
    }
    
    // Test with struct
    typedef struct {
        int id;
        float value;
        char name[16];
    } TestStruct;
    
    SamrenaVector *struct_vec = samrena_vector_init_owned(sizeof(TestStruct), 2);
    TestStruct structs[] = {
        {1, 1.5f, "First"},
        {2, 2.5f, "Second"}
    };
    
    for (int i = 0; i < 2; i++) {
        TestStruct *result = (TestStruct*)samrena_vector_push(struct_vec, &structs[i]);
        assert(result != NULL);
        assert(result->id == structs[i].id);
        assert(result->value == structs[i].value);
        assert(strcmp(result->name, structs[i].name) == 0);
    }
    
    samrena_vector_destroy(char_vec);
    samrena_vector_destroy(double_vec);
    samrena_vector_destroy(struct_vec);
    printf("PASSED\n");
}

void test_vector_pop_basic() {
    printf("Testing samrena_vector_pop basic functionality... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    
    // Push elements
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    assert(vec->size == 5);
    
    // Pop elements in LIFO order
    for (int i = 4; i >= 0; i--) {
        int *popped = (int*)samrena_vector_pop(vec);
        assert(popped != NULL);
        assert(*popped == values[i]);
        assert(vec->size == (uint64_t)i);
    }
    
    assert(samrena_vector_is_empty(vec) == true);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_pop_empty() {
    printf("Testing samrena_vector_pop with empty vector... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    // Pop from empty vector should return NULL
    void *result = samrena_vector_pop(vec);
    assert(result == NULL);
    assert(vec->size == 0);
    
    // Add one element and pop it
    int value = 42;
    samrena_vector_push(vec, &value);
    assert(vec->size == 1);
    
    int *popped = (int*)samrena_vector_pop(vec);
    assert(popped != NULL);
    assert(*popped == 42);
    assert(vec->size == 0);
    
    // Pop again from now-empty vector
    result = samrena_vector_pop(vec);
    assert(result == NULL);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_pop_null() {
    printf("Testing samrena_vector_pop with NULL vector... ");
    
    assert(samrena_vector_pop(NULL) == NULL);
    
    printf("PASSED\n");
}

void test_vector_push_pop_cycle() {
    printf("Testing samrena_vector_push/pop cycles... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 3);
    assert(vec != NULL);
    
    // Test multiple push/pop cycles
    for (int cycle = 0; cycle < 5; cycle++) {
        // Push values
        for (int i = 0; i < 3; i++) {
            int value = cycle * 10 + i;
            void *result = samrena_vector_push(vec, &value);
            assert(result != NULL);
            assert(*(int*)result == value);
        }
        
        assert(vec->size == 3);
        
        // Pop values
        for (int i = 2; i >= 0; i--) {
            int *popped = (int*)samrena_vector_pop(vec);
            assert(popped != NULL);
            assert(*popped == cycle * 10 + i);
        }
        
        assert(vec->size == 0);
        assert(samrena_vector_is_empty(vec) == true);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_growth_factor() {
    printf("Testing vector growth factor behavior... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 2);
    assert(vec != NULL);
    assert(vec->capacity == 2);
    assert(vec->growth_factor > 1.0f);
    
    // Track capacity changes
    size_t capacities[10];
    int capacity_count = 0;
    capacities[capacity_count++] = vec->capacity;
    
    // Push elements to trigger multiple growths
    for (int i = 0; i < 20; i++) {
        size_t old_capacity = vec->capacity;
        samrena_vector_push(vec, &i);
        
        // If capacity changed, record it
        if (vec->capacity != old_capacity) {
            assert(vec->capacity > old_capacity);
            assert(capacity_count < 10);
            capacities[capacity_count++] = vec->capacity;
            
            // Check that growth is reasonable (not just +1)
            assert(vec->capacity >= (size_t)(old_capacity * vec->growth_factor));
        }
    }
    
    // Should have grown at least a few times
    assert(capacity_count > 2);
    
    // Verify all elements are correct
    for (int i = 0; i < 20; i++) {
        int retrieved;
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == i);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_min_growth() {
    printf("Testing vector minimum growth behavior... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 1);
    assert(vec != NULL);
    assert(vec->min_growth > 0);
    
    size_t initial_capacity = vec->capacity;
    
    // Fill to capacity
    for (size_t i = 0; i < initial_capacity; i++) {
        int value = (int)i;
        samrena_vector_push(vec, &value);
    }
    
    // Push one more to trigger growth
    int trigger_value = 999;
    samrena_vector_push(vec, &trigger_value);
    
    // New capacity should be at least old_capacity + min_growth
    size_t expected_min_capacity = initial_capacity + vec->min_growth;
    assert(vec->capacity >= expected_min_capacity);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_large_operations() {
    printf("Testing vector operations with large number of elements... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    const int num_elements = 1000;
    
    // Push many elements
    for (int i = 0; i < num_elements; i++) {
        void *result = samrena_vector_push(vec, &i);
        assert(result != NULL);
        
        // Occasionally verify the element was stored correctly
        if (i % 100 == 0) {
            assert(*(int*)result == i);
        }
    }
    
    assert(vec->size == num_elements);
    
    // Verify some elements randomly
    int indices[] = {0, 1, 50, 100, 250, 500, 750, 999};
    for (size_t i = 0; i < sizeof(indices)/sizeof(indices[0]); i++) {
        int retrieved;
        assert(samrena_vector_get(vec, indices[i], &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == indices[i]);
    }
    
    // Pop all elements
    for (int i = num_elements - 1; i >= 0; i--) {
        int *popped = (int*)samrena_vector_pop(vec);
        assert(popped != NULL);
        assert(*popped == i);
        
        // Occasionally verify size
        if (i % 100 == 0) {
            assert(vec->size == (uint64_t)i);
        }
    }
    
    assert(samrena_vector_is_empty(vec) == true);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_operations_interleaved() {
    printf("Testing interleaved push/pop operations... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    // Pattern: push 3, pop 1, push 2, pop 2, push 4, pop all
    
    // Push 3
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec, &i);
    }
    assert(vec->size == 3);
    
    // Pop 1
    int *popped = (int*)samrena_vector_pop(vec);
    assert(popped != NULL && *popped == 2);
    assert(vec->size == 2);
    
    // Push 2 more
    int values[] = {10, 11};
    for (int i = 0; i < 2; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    assert(vec->size == 4);
    
    // Pop 2
    popped = (int*)samrena_vector_pop(vec);
    assert(popped != NULL && *popped == 11);
    popped = (int*)samrena_vector_pop(vec);
    assert(popped != NULL && *popped == 10);
    assert(vec->size == 2);
    
    // Push 4 more
    int more_values[] = {20, 21, 22, 23};
    for (int i = 0; i < 4; i++) {
        samrena_vector_push(vec, &more_values[i]);
    }
    assert(vec->size == 6);
    
    // Pop all remaining
    int expected[] = {23, 22, 21, 20, 1, 0};
    for (int i = 0; i < 6; i++) {
        popped = (int*)samrena_vector_pop(vec);
        assert(popped != NULL);
        assert(*popped == expected[i]);
    }
    
    assert(samrena_vector_is_empty(vec) == true);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_operations_stability() {
    printf("Testing vector operations stability after growth... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 2);
    assert(vec != NULL);
    
    // Push elements to force multiple growths
    int values[50];
    for (int i = 0; i < 50; i++) {
        values[i] = i * i; // Use squares for more interesting data
        samrena_vector_push(vec, &values[i]);
    }
    
    // Verify all elements are still correct after growth
    for (int i = 0; i < 50; i++) {
        int retrieved;
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == values[i]);
    }
    
    // Pop half the elements
    for (int i = 0; i < 25; i++) {
        int *popped = (int*)samrena_vector_pop(vec);
        assert(popped != NULL);
        assert(*popped == values[49 - i]);
    }
    
    assert(vec->size == 25);
    
    // Verify remaining elements are still correct
    for (int i = 0; i < 25; i++) {
        int retrieved;
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == values[i]);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

int main() {
    printf("Starting samrena vector operations tests...\n\n");

    test_vector_push_basic();
    test_vector_push_growth();
    test_vector_push_null_cases();
    test_vector_push_different_types();
    test_vector_pop_basic();
    test_vector_pop_empty();
    test_vector_pop_null();
    test_vector_push_pop_cycle();
    test_vector_growth_factor();
    test_vector_min_growth();
    test_vector_large_operations();
    test_vector_operations_interleaved();
    test_vector_operations_stability();

    printf("\nAll samrena vector operations tests passed!\n");
    return 0;
}
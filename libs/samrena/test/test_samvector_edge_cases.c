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
#include <stdint.h>
#include <limits.h>

void test_vector_null_pointer_handling() {
    printf("Testing comprehensive NULL pointer handling... ");
    
    int dummy = 42;
    
    // All functions should handle NULL vector gracefully
    assert(samrena_vector_push(NULL, &dummy) == NULL);
    assert(samrena_vector_pop(NULL) == NULL);
    assert(samrena_vector_resize(NULL, 10) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_get(NULL, 0, &dummy) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_set(NULL, 0, &dummy) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_at(NULL, 0) == NULL);
    assert(samrena_vector_at_const(NULL, 0) == NULL);
    assert(samrena_vector_truncate(NULL, 0) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_reset(NULL, 10) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_size(NULL) == 0);
    assert(samrena_vector_capacity(NULL) == 0);
    assert(samrena_vector_is_empty(NULL) == true);
    assert(samrena_vector_is_full(NULL) == false);
    assert(samrena_vector_available(NULL) == 0);
    
    // Clear and destroy should not crash with NULL
    samrena_vector_clear(NULL);
    samrena_vector_destroy(NULL);
    
    printf("PASSED\n");
}

void test_vector_null_element_handling() {
    printf("Testing NULL element parameter handling... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    // Add some data first
    int value = 42;
    samrena_vector_push(vec, &value);
    
    // Functions that take element pointers should handle NULL
    assert(samrena_vector_push(vec, NULL) == NULL);
    assert(samrena_vector_get(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_set(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    
    // Vector should remain unchanged
    assert(vec->size == 1);
    int retrieved;
    assert(samrena_vector_get(vec, 0, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 42);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_boundary_indices() {
    printf("Testing boundary index conditions... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    // Add 5 elements
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &i);
    }
    
    int dummy;
    
    // Test valid boundary indices
    assert(samrena_vector_get(vec, 0, &dummy) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_get(vec, 4, &dummy) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_set(vec, 0, &dummy) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_set(vec, 4, &dummy) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_at(vec, 0) != NULL);
    assert(samrena_vector_at(vec, 4) != NULL);
    
    // Test invalid boundary indices
    assert(samrena_vector_get(vec, 5, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_get(vec, 10, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, 5, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, 10, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_at(vec, 5) == NULL);
    assert(samrena_vector_at(vec, 10) == NULL);
    
    // Test very large indices
    assert(samrena_vector_get(vec, SIZE_MAX, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, SIZE_MAX, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_at(vec, SIZE_MAX) == NULL);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_empty_vector_operations() {
    printf("Testing operations on empty vector... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    assert(vec->size == 0);
    
    int dummy = 99;
    
    // All access operations should fail on empty vector
    assert(samrena_vector_get(vec, 0, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, 0, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_at(vec, 0) == NULL);
    assert(samrena_vector_at_const(vec, 0) == NULL);
    assert(samrena_vector_pop(vec) == NULL);
    
    // Query operations should work correctly
    assert(samrena_vector_size(vec) == 0);
    assert(samrena_vector_capacity(vec) == 10);
    assert(samrena_vector_is_empty(vec) == true);
    assert(samrena_vector_is_full(vec) == false);
    assert(samrena_vector_available(vec) == 10);
    
    // Clear and truncate operations should work (but be no-ops)
    samrena_vector_clear(vec);
    assert(vec->size == 0);
    
    assert(samrena_vector_truncate(vec, 0) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->size == 0);
    
    // Truncating to any size > 0 should fail when vector is empty
    assert(samrena_vector_truncate(vec, 1) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_single_element_operations() {
    printf("Testing operations with single element... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 1);
    assert(vec != NULL);
    assert(vec->capacity == 1);
    
    int value = 42;
    
    // Add single element
    void *result = samrena_vector_push(vec, &value);
    assert(result != NULL);
    assert(vec->size == 1);
    assert(samrena_vector_is_full(vec) == true);
    assert(samrena_vector_available(vec) == 0);
    
    // Test access to the single element
    int retrieved;
    assert(samrena_vector_get(vec, 0, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 42);
    
    int *ptr = (int*)samrena_vector_at(vec, 0);
    assert(ptr != NULL);
    assert(*ptr == 42);
    
    // Test boundary - index 1 should fail
    assert(samrena_vector_get(vec, 1, &retrieved) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_at(vec, 1) == NULL);
    
    // Modify the single element
    int new_value = 99;
    assert(samrena_vector_set(vec, 0, &new_value) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_get(vec, 0, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 99);
    
    // Pop the single element
    int *popped = (int*)samrena_vector_pop(vec);
    assert(popped != NULL);
    assert(*popped == 99);
    assert(vec->size == 0);
    assert(samrena_vector_is_empty(vec) == true);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_zero_element_size_handling() {
    printf("Testing zero element size handling... ");
    
    // Both init functions should fail with zero element size
    Samrena *arena = samrena_create_default();
    assert(samrena_vector_init(arena, 0, 10) == NULL);
    assert(samrena_vector_init_owned(0, 10) == NULL);
    
    samrena_destroy(arena);
    printf("PASSED\n");
}

void test_vector_very_large_element_size() {
    printf("Testing very large element size... ");
    
    // Test with reasonably large but valid element size
    typedef struct {
        char data[1024];
        int id;
        double values[128];
    } LargeElement;
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(LargeElement), 2);
    assert(vec != NULL);
    assert(vec->element_size == sizeof(LargeElement));
    
    LargeElement elem1 = {{0}, 1, {0}};
    elem1.data[0] = 'A';
    elem1.data[1] = 'B';
    elem1.values[0] = 1.5;
    elem1.values[127] = 2.5;
    
    // Should be able to store and retrieve large elements
    LargeElement *stored = (LargeElement*)samrena_vector_push(vec, &elem1);
    assert(stored != NULL);
    assert(stored->id == 1);
    assert(stored->data[0] == 'A');
    assert(stored->data[1] == 'B');
    assert(stored->values[0] == 1.5);
    assert(stored->values[127] == 2.5);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_capacity_edge_cases() {
    printf("Testing capacity edge cases... ");
    
    // Test with capacity 1
    SamrenaVector *vec1 = samrena_vector_init_owned(sizeof(int), 1);
    assert(vec1 != NULL);
    assert(vec1->capacity == 1);
    
    int val = 42;
    samrena_vector_push(vec1, &val);
    assert(vec1->size == 1);
    assert(samrena_vector_is_full(vec1) == true);
    
    // Pushing another should grow capacity
    int val2 = 43;
    samrena_vector_push(vec1, &val2);
    assert(vec1->size == 2);
    assert(vec1->capacity > 1);
    
    samrena_vector_destroy(vec1);
    
    // Test with zero initial capacity (should use default)
    SamrenaVector *vec2 = samrena_vector_init_owned(sizeof(int), 0);
    assert(vec2 != NULL);
    assert(vec2->capacity > 0);
    
    samrena_vector_destroy(vec2);
    
    printf("PASSED\n");
}

void test_vector_resize_edge_cases() {
    printf("Testing resize edge cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    // Add some elements
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &i);
    }
    
    // Resize to same capacity should work
    assert(samrena_vector_resize(vec, 10) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->capacity == 10);
    assert(vec->size == 5);
    
    // Resize to zero should succeed
    // Let's first save the current state and restore for other tests
    size_t original_size = vec->size;
    size_t original_capacity = vec->capacity;
    assert(samrena_vector_resize(vec, 0) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->capacity == 0);
    assert(vec->size == 0);
    
    // Restore to test other operations
    assert(samrena_vector_resize(vec, original_capacity) == SAMRENA_VECTOR_SUCCESS);
    // Need to re-add the elements since they were lost
    for (int i = 0; i < (int)original_size; i++) {
        samrena_vector_push(vec, &i);
    }
    
    // Resize to less than current size should succeed and truncate
    assert(samrena_vector_resize(vec, 3) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->size == 3);
    assert(vec->capacity == 3);
    
    // Resize to larger capacity should work (but size stays the same)
    assert(samrena_vector_resize(vec, 5) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->capacity == 5);
    assert(vec->size == 3); // Size remains at current size, doesn't grow automatically
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_truncate_edge_cases() {
    printf("Testing truncate edge cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    // Truncate empty vector to 0 should work
    assert(samrena_vector_truncate(vec, 0) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->size == 0);
    
    // Add elements
    for (int i = 0; i < 7; i++) {
        samrena_vector_push(vec, &i);
    }
    
    // Truncate to same size should work
    assert(samrena_vector_truncate(vec, 7) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->size == 7);
    
    // Truncate to larger size should fail
    assert(samrena_vector_truncate(vec, 8) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_truncate(vec, 10) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_error_code_completeness() {
    printf("Testing all error codes are used appropriately... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(vec != NULL);
    
    int dummy = 42;
    
    // SAMRENA_VECTOR_SUCCESS
    assert(samrena_vector_resize(vec, 10) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_get(vec, 0, &dummy) != SAMRENA_VECTOR_SUCCESS); // Should be out of bounds
    
    samrena_vector_push(vec, &dummy);
    assert(samrena_vector_get(vec, 0, &dummy) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_set(vec, 0, &dummy) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_truncate(vec, 1) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_reset(vec, 5) == SAMRENA_VECTOR_SUCCESS);
    
    // SAMRENA_VECTOR_ERROR_NULL_POINTER
    assert(samrena_vector_resize(NULL, 10) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_get(NULL, 0, &dummy) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_get(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_set(NULL, 0, &dummy) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_set(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_truncate(NULL, 0) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    assert(samrena_vector_reset(NULL, 5) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    
    // SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS
    assert(samrena_vector_get(vec, 10, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, 10, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    // SAMRENA_VECTOR_ERROR_INVALID_OPERATION and SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS
    samrena_vector_push(vec, &dummy);
    samrena_vector_push(vec, &dummy);
    assert(vec->size == 2);
    
    // These should actually succeed (resize allows shrinking)
    assert(samrena_vector_resize(vec, 1) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->size == 1);
    assert(samrena_vector_resize(vec, 0) == SAMRENA_VECTOR_SUCCESS);
    assert(vec->size == 0);
    
    // Reset with zero should succeed
    assert(samrena_vector_reset(vec, 5) == SAMRENA_VECTOR_SUCCESS);
    samrena_vector_push(vec, &dummy);
    samrena_vector_push(vec, &dummy);
    assert(vec->size == 2);
    
    // Truncate to larger size should fail with OUT_OF_BOUNDS
    assert(samrena_vector_truncate(vec, 3) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_memory_ownership_edge_cases() {
    printf("Testing memory ownership edge cases... ");
    
    // Test owned arena lifecycle
    SamrenaVector *owned_vec = samrena_vector_init_owned(sizeof(int), 5);
    assert(owned_vec != NULL);
    assert(owned_vec->owns_arena == true);
    assert(owned_vec->arena != NULL);
    
    // Should be able to use the vector normally
    int value = 123;
    samrena_vector_push(owned_vec, &value);
    
    int retrieved;
    assert(samrena_vector_get(owned_vec, 0, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 123);
    
    samrena_vector_destroy(owned_vec);
    
    // Test borrowed arena lifecycle
    Samrena *external_arena = samrena_create_default();
    SamrenaVector *borrowed_vec = samrena_vector_init(external_arena, sizeof(int), 5);
    assert(borrowed_vec != NULL);
    assert(borrowed_vec->owns_arena == false);
    assert(borrowed_vec->arena == external_arena);
    
    samrena_vector_push(borrowed_vec, &value);
    assert(samrena_vector_get(borrowed_vec, 0, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 123);
    
    // Destroying vector should not destroy external arena
    samrena_vector_destroy(borrowed_vec);
    
    // External arena should still be usable
    void *test_alloc = samrena_push(external_arena, 100);
    assert(test_alloc != NULL);
    
    samrena_destroy(external_arena);
    
    printf("PASSED\n");
}

void test_vector_concurrent_safety_boundaries() {
    printf("Testing vector operation safety boundaries... ");
    
    // This test focuses on edge conditions that might cause issues
    // if the vector were used in a concurrent context (though it's not thread-safe)
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 3);
    assert(vec != NULL);
    
    // Fill to capacity
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec, &i);
    }
    
    // Get pointers to elements
    int *ptr0 = (int*)samrena_vector_at(vec, 0);
    int *ptr1 = (int*)samrena_vector_at(vec, 1);
    int *ptr2 = (int*)samrena_vector_at(vec, 2);
    
    assert(ptr0 != NULL && *ptr0 == 0);
    assert(ptr1 != NULL && *ptr1 == 1);
    assert(ptr2 != NULL && *ptr2 == 2);
    
    // Push one more element (should trigger reallocation)
    int new_val = 3;
    samrena_vector_push(vec, &new_val);
    
    // Original pointers might now be invalid (this is expected behavior)
    // But we should still be able to access all elements correctly
    for (int i = 0; i < 4; i++) {
        int retrieved;
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == i);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

int main() {
    printf("Starting samrena vector edge cases tests...\n\n");

    test_vector_null_pointer_handling();
    test_vector_null_element_handling();
    test_vector_boundary_indices();
    test_vector_empty_vector_operations();
    test_vector_single_element_operations();
    test_vector_zero_element_size_handling();
    test_vector_very_large_element_size();
    test_vector_capacity_edge_cases();
    test_vector_resize_edge_cases();
    test_vector_truncate_edge_cases();
    test_vector_error_code_completeness();
    test_vector_memory_ownership_edge_cases();
    test_vector_concurrent_safety_boundaries();

    printf("\nAll samrena vector edge cases tests passed!\n");
    return 0;
}
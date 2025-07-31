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

void test_vector_init_basic() {
    printf("Testing samrena_vector_init basic functionality... ");
    
    Samrena *arena = samrena_create_default();
    assert(arena != NULL);
    
    SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 10);
    assert(vec != NULL);
    assert(vec->size == 0);
    assert(vec->element_size == sizeof(int));
    assert(vec->capacity == 10);
    assert(vec->data != NULL);
    assert(vec->arena == arena);
    assert(vec->owns_arena == false);
    assert(vec->growth_factor > 1.0f);
    assert(vec->min_growth > 0);
    
    samrena_destroy(arena);
    printf("PASSED\n");
}

void test_vector_init_zero_capacity() {
    printf("Testing samrena_vector_init with zero capacity... ");
    
    Samrena *arena = samrena_create_default();
    SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 0);
    
    assert(vec != NULL);
    assert(vec->capacity > 0); // Should use default minimum capacity
    assert(vec->element_size == sizeof(int));
    assert(vec->size == 0);
    
    samrena_destroy(arena);
    printf("PASSED\n");
}

void test_vector_init_large_element() {
    printf("Testing samrena_vector_init with large elements... ");
    
    typedef struct {
        char buffer[1024];
        int id;
        double value;
    } LargeStruct;
    
    Samrena *arena = samrena_create_default();
    SamrenaVector *vec = samrena_vector_init(arena, sizeof(LargeStruct), 5);
    
    assert(vec != NULL);
    assert(vec->element_size == sizeof(LargeStruct));
    assert(vec->capacity == 5);
    assert(vec->size == 0);
    
    samrena_destroy(arena);
    printf("PASSED\n");
}

void test_vector_init_null_arena() {
    printf("Testing samrena_vector_init with NULL arena... ");
    
    SamrenaVector *vec = samrena_vector_init(NULL, sizeof(int), 10);
    assert(vec == NULL);
    
    printf("PASSED\n");
}

void test_vector_init_zero_element_size() {
    printf("Testing samrena_vector_init with zero element size... ");
    
    Samrena *arena = samrena_create_default();
    SamrenaVector *vec = samrena_vector_init(arena, 0, 10);
    assert(vec == NULL);
    
    samrena_destroy(arena);
    printf("PASSED\n");
}

void test_vector_init_owned_basic() {
    printf("Testing samrena_vector_init_owned basic functionality... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    assert(vec->size == 0);
    assert(vec->element_size == sizeof(int));
    assert(vec->capacity == 10);
    assert(vec->data != NULL);
    assert(vec->arena != NULL);
    assert(vec->owns_arena == true);
    assert(vec->growth_factor > 1.0f);
    assert(vec->min_growth > 0);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_init_owned_zero_capacity() {
    printf("Testing samrena_vector_init_owned with zero capacity... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 0);
    assert(vec != NULL);
    assert(vec->capacity > 0); // Should use default minimum capacity
    assert(vec->owns_arena == true);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_init_owned_zero_element_size() {
    printf("Testing samrena_vector_init_owned with zero element size... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(0, 10);
    assert(vec == NULL);
    
    printf("PASSED\n");
}

void test_vector_init_owned_large_capacity() {
    printf("Testing samrena_vector_init_owned with large capacity... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(double), 100000);
    assert(vec != NULL);
    assert(vec->capacity == 100000);
    assert(vec->element_size == sizeof(double));
    assert(vec->owns_arena == true);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_destroy_owned() {
    printf("Testing samrena_vector_destroy with owned arena... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    // Add some data to make sure it's properly cleaned up
    int values[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    assert(vec->size == 5);
    
    samrena_vector_destroy(vec);
    // If we reach here without crashing, destroy worked correctly
    
    printf("PASSED\n");
}

void test_vector_destroy_borrowed() {
    printf("Testing samrena_vector_destroy with borrowed arena... ");
    
    Samrena *arena = samrena_create_default();
    SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 10);
    assert(vec != NULL);
    assert(vec->owns_arena == false);
    
    // Add some data
    int values[] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    samrena_vector_destroy(vec);
    
    // Arena should still be valid since vector didn't own it
    // We can still use the arena for other allocations
    void *test_alloc = samrena_push(arena, 100);
    assert(test_alloc != NULL);
    
    samrena_destroy(arena);
    printf("PASSED\n");
}

void test_vector_destroy_null() {
    printf("Testing samrena_vector_destroy with NULL... ");
    
    // Should handle NULL gracefully without crashing
    samrena_vector_destroy(NULL);
    
    printf("PASSED\n");
}

void test_vector_lifecycle_multiple() {
    printf("Testing multiple vector lifecycle operations... ");
    
    // Test multiple owned vectors
    SamrenaVector *vec1 = samrena_vector_init_owned(sizeof(int), 5);
    SamrenaVector *vec2 = samrena_vector_init_owned(sizeof(double), 3);
    SamrenaVector *vec3 = samrena_vector_init_owned(sizeof(char), 20);
    
    assert(vec1 != NULL && vec2 != NULL && vec3 != NULL);
    assert(vec1->arena != vec2->arena);
    assert(vec2->arena != vec3->arena);
    
    // Add data to each
    int int_val = 42;
    samrena_vector_push(vec1, &int_val);
    
    double double_val = 3.14;
    samrena_vector_push(vec2, &double_val);
    
    char char_val = 'A';
    samrena_vector_push(vec3, &char_val);
    
    assert(vec1->size == 1);
    assert(vec2->size == 1);
    assert(vec3->size == 1);
    
    // Destroy in different order
    samrena_vector_destroy(vec2);
    samrena_vector_destroy(vec1);
    samrena_vector_destroy(vec3);
    
    printf("PASSED\n");
}

void test_vector_lifecycle_shared_arena() {
    printf("Testing multiple vectors with shared arena... ");
    
    Samrena *shared_arena = samrena_create_default();
    
    // Create multiple vectors using the same arena
    SamrenaVector *vec1 = samrena_vector_init(shared_arena, sizeof(int), 5);
    SamrenaVector *vec2 = samrena_vector_init(shared_arena, sizeof(float), 8);
    SamrenaVector *vec3 = samrena_vector_init(shared_arena, sizeof(short), 12);
    
    assert(vec1 != NULL && vec2 != NULL && vec3 != NULL);
    assert(vec1->arena == shared_arena);
    assert(vec2->arena == shared_arena);
    assert(vec3->arena == shared_arena);
    assert(vec1->owns_arena == false);
    assert(vec2->owns_arena == false);
    assert(vec3->owns_arena == false);
    
    // Add data to ensure allocations work
    int int_vals[] = {1, 2, 3};
    float float_vals[] = {1.1f, 2.2f, 3.3f, 4.4f};
    short short_vals[] = {10, 20};
    
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec1, &int_vals[i]);
    }
    for (int i = 0; i < 4; i++) {
        samrena_vector_push(vec2, &float_vals[i]);
    }
    for (int i = 0; i < 2; i++) {
        samrena_vector_push(vec3, &short_vals[i]);
    }
    
    assert(vec1->size == 3);
    assert(vec2->size == 4);
    assert(vec3->size == 2);
    
    // Destroy vectors (arena should remain valid)
    samrena_vector_destroy(vec1);
    samrena_vector_destroy(vec2);
    samrena_vector_destroy(vec3);
    
    // Arena should still be usable
    void *test_alloc = samrena_push(shared_arena, 64);
    assert(test_alloc != NULL);
    
    samrena_destroy(shared_arena);
    printf("PASSED\n");
}

int main() {
    printf("Starting samrena vector lifecycle tests...\n\n");

    test_vector_init_basic();
    test_vector_init_zero_capacity();
    test_vector_init_large_element();
    test_vector_init_null_arena();
    test_vector_init_zero_element_size();
    
    test_vector_init_owned_basic();
    test_vector_init_owned_zero_capacity();
    test_vector_init_owned_zero_element_size();
    test_vector_init_owned_large_capacity();
    
    test_vector_destroy_owned();
    test_vector_destroy_borrowed();
    test_vector_destroy_null();
    
    test_vector_lifecycle_multiple();
    test_vector_lifecycle_shared_arena();

    printf("\nAll samrena vector lifecycle tests passed!\n");
    return 0;
}
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

void test_vector_get_success() {
    printf("Testing samrena_vector_get success cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    int retrieved;
    for (int i = 0; i < 5; i++) {
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == values[i]);
    }
    
    // Test first and last elements specifically
    assert(samrena_vector_get(vec, 0, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 10);
    assert(samrena_vector_get(vec, 4, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 50);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_get_error_cases() {
    printf("Testing samrena_vector_get error cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    int retrieved;
    
    // Test NULL vector
    assert(samrena_vector_get(NULL, 0, &retrieved) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    
    // Test NULL out_element
    assert(samrena_vector_get(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    
    // Test out of bounds
    assert(samrena_vector_get(vec, 3, &retrieved) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_get(vec, 10, &retrieved) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_get(vec, SIZE_MAX, &retrieved) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    // Test with empty vector
    samrena_vector_clear(vec);
    assert(samrena_vector_get(vec, 0, &retrieved) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_set_success() {
    printf("Testing samrena_vector_set success cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int original_values[] = {10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &original_values[i]);
    }
    
    // Set new values
    int new_values[] = {100, 200, 300, 400, 500};
    for (int i = 0; i < 5; i++) {
        assert(samrena_vector_set(vec, i, &new_values[i]) == SAMRENA_VECTOR_SUCCESS);
    }
    
    // Verify new values
    int retrieved;
    for (int i = 0; i < 5; i++) {
        assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == new_values[i]);
    }
    
    // Test modifying specific indices
    int special_value = 999;
    assert(samrena_vector_set(vec, 2, &special_value) == SAMRENA_VECTOR_SUCCESS);
    assert(samrena_vector_get(vec, 2, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 999);
    
    // Other elements should remain unchanged
    assert(samrena_vector_get(vec, 1, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 200);
    assert(samrena_vector_get(vec, 3, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == 400);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_set_error_cases() {
    printf("Testing samrena_vector_set error cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    int new_value = 999;
    
    // Test NULL vector
    assert(samrena_vector_set(NULL, 0, &new_value) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    
    // Test NULL element
    assert(samrena_vector_set(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
    
    // Test out of bounds
    assert(samrena_vector_set(vec, 3, &new_value) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, 10, &new_value) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, SIZE_MAX, &new_value) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    // Test with empty vector
    samrena_vector_clear(vec);
    assert(samrena_vector_set(vec, 0, &new_value) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_at_success() {
    printf("Testing samrena_vector_at success cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {111, 222, 333, 444, 555};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    // Test direct pointer access
    for (int i = 0; i < 5; i++) {
        int *ptr = (int*)samrena_vector_at(vec, i);
        assert(ptr != NULL);
        assert(*ptr == values[i]);
        
        // Test that we can modify through the pointer
        *ptr = values[i] + 1000;
        
        // Verify modification
        int *verify_ptr = (int*)samrena_vector_at(vec, i);
        assert(*verify_ptr == values[i] + 1000);
    }
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_at_error_cases() {
    printf("Testing samrena_vector_at error cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    // Test NULL vector
    assert(samrena_vector_at(NULL, 0) == NULL);
    
    // Test out of bounds
    assert(samrena_vector_at(vec, 3) == NULL);
    assert(samrena_vector_at(vec, 10) == NULL);
    assert(samrena_vector_at(vec, SIZE_MAX) == NULL);
    
    // Test with empty vector
    samrena_vector_clear(vec);
    assert(samrena_vector_at(vec, 0) == NULL);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_at_const_success() {
    printf("Testing samrena_vector_at_const success cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(double), 10);
    assert(vec != NULL);
    
    double values[] = {1.1, 2.2, 3.3, 4.4, 5.5};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    // Test const pointer access
    for (int i = 0; i < 5; i++) {
        const double *ptr = (const double*)samrena_vector_at_const(vec, i);
        assert(ptr != NULL);
        assert(*ptr == values[i]);
    }
    
    // Test that both const and non-const versions return same pointer
    const double *const_ptr = (const double*)samrena_vector_at_const(vec, 0);
    double *non_const_ptr = (double*)samrena_vector_at(vec, 0);
    assert(const_ptr == non_const_ptr);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_at_const_error_cases() {
    printf("Testing samrena_vector_at_const error cases... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    // Test NULL vector
    assert(samrena_vector_at_const(NULL, 0) == NULL);
    
    // Test out of bounds
    assert(samrena_vector_at_const(vec, 3) == NULL);
    assert(samrena_vector_at_const(vec, 10) == NULL);
    assert(samrena_vector_at_const(vec, SIZE_MAX) == NULL);
    
    // Test with empty vector
    samrena_vector_clear(vec);
    assert(samrena_vector_at_const(vec, 0) == NULL);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_unchecked_access() {
    printf("Testing samrena_vector_at_unchecked functions... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {123, 456, 789, 321, 654};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    // Test unchecked access (should work without bounds checking)
    for (int i = 0; i < 5; i++) {
        int *ptr = (int*)samrena_vector_at_unchecked(vec, i);
        assert(*ptr == values[i]);
        
        const int *const_ptr = (const int*)samrena_vector_at_unchecked_const(vec, i);
        assert(*const_ptr == values[i]);
        assert(ptr == const_ptr);
    }
    
    // Test that we can modify through unchecked access
    int *first = (int*)samrena_vector_at_unchecked(vec, 0);
    *first = 999;
    assert(*(int*)samrena_vector_at_unchecked(vec, 0) == 999);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_elem_macro() {
    printf("Testing SAMRENA_VECTOR_ELEM macro... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    
    int values[] = {11, 22, 33, 44, 55};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(vec, &values[i]);
    }
    
    // Test reading through macro
    for (int i = 0; i < 5; i++) {
        assert(SAMRENA_VECTOR_ELEM(vec, int, i) == values[i]);
    }
    
    // Test writing through macro
    SAMRENA_VECTOR_ELEM(vec, int, 0) = 1111;
    SAMRENA_VECTOR_ELEM(vec, int, 2) = 3333;
    SAMRENA_VECTOR_ELEM(vec, int, 4) = 5555;
    
    assert(SAMRENA_VECTOR_ELEM(vec, int, 0) == 1111);
    assert(SAMRENA_VECTOR_ELEM(vec, int, 1) == 22);
    assert(SAMRENA_VECTOR_ELEM(vec, int, 2) == 3333);
    assert(SAMRENA_VECTOR_ELEM(vec, int, 3) == 44);
    assert(SAMRENA_VECTOR_ELEM(vec, int, 4) == 5555);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

void test_vector_access_different_types() {
    printf("Testing vector access with different data types... ");
    
    // Test with char
    SamrenaVector *char_vec = samrena_vector_init_owned(sizeof(char), 5);
    char chars[] = {'A', 'B', 'C', 'D', 'E'};
    for (int i = 0; i < 5; i++) {
        samrena_vector_push(char_vec, &chars[i]);
    }
    
    for (int i = 0; i < 5; i++) {
        char retrieved;
        assert(samrena_vector_get(char_vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved == chars[i]);
        
        char *ptr = (char*)samrena_vector_at(char_vec, i);
        assert(*ptr == chars[i]);
        
        assert(SAMRENA_VECTOR_ELEM(char_vec, char, i) == chars[i]);
    }
    
    // Test with struct
    typedef struct {
        int id;
        float value;
        char name[16];
    } TestStruct;
    
    SamrenaVector *struct_vec = samrena_vector_init_owned(sizeof(TestStruct), 3);
    TestStruct test_structs[] = {
        {1, 1.1f, "First"},
        {2, 2.2f, "Second"},
        {3, 3.3f, "Third"}
    };
    
    for (int i = 0; i < 3; i++) {
        samrena_vector_push(struct_vec, &test_structs[i]);
    }
    
    for (int i = 0; i < 3; i++) {
        TestStruct retrieved;
        assert(samrena_vector_get(struct_vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
        assert(retrieved.id == test_structs[i].id);
        assert(retrieved.value == test_structs[i].value);
        assert(strcmp(retrieved.name, test_structs[i].name) == 0);
        
        TestStruct *ptr = (TestStruct*)samrena_vector_at(struct_vec, i);
        assert(ptr->id == test_structs[i].id);
        assert(ptr->value == test_structs[i].value);
        assert(strcmp(ptr->name, test_structs[i].name) == 0);
    }
    
    // Test modification through pointer
    TestStruct *first_struct = (TestStruct*)samrena_vector_at(struct_vec, 0);
    first_struct->id = 999;
    strcpy(first_struct->name, "Modified");
    
    TestStruct verify;
    assert(samrena_vector_get(struct_vec, 0, &verify) == SAMRENA_VECTOR_SUCCESS);
    assert(verify.id == 999);
    assert(strcmp(verify.name, "Modified") == 0);
    
    samrena_vector_destroy(char_vec);
    samrena_vector_destroy(struct_vec);
    printf("PASSED\n");
}

void test_vector_access_empty_vector() {
    printf("Testing vector access with empty vector... ");
    
    SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
    assert(vec != NULL);
    assert(vec->size == 0);
    
    int dummy;
    
    // All access functions should fail/return NULL for empty vector
    assert(samrena_vector_get(vec, 0, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_set(vec, 0, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
    assert(samrena_vector_at(vec, 0) == NULL);
    assert(samrena_vector_at_const(vec, 0) == NULL);
    
    samrena_vector_destroy(vec);
    printf("PASSED\n");
}

int main() {
    printf("Starting samrena vector access tests...\n\n");

    test_vector_get_success();
    test_vector_get_error_cases();
    test_vector_set_success();
    test_vector_set_error_cases();
    test_vector_at_success();
    test_vector_at_error_cases();
    test_vector_at_const_success();
    test_vector_at_const_error_cases();
    test_vector_unchecked_access();
    test_vector_elem_macro();
    test_vector_access_different_types();
    test_vector_access_empty_vector();

    printf("\nAll samrena vector access tests passed!\n");
    return 0;
}
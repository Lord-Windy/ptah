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

// Test vector creation and basic operations
void test_vector_init() {
  printf("Testing vector initialization... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 10);

  assert(vec != NULL);
  assert(vec->size == 0);
  assert(vec->element_size == sizeof(int));
  assert(vec->capacity == 10);
  assert(vec->data != NULL);

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test pushing elements to the vector
void test_vector_push() {
  printf("Testing vector push... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 3);

  int values[] = {10, 20, 30, 40, 50};

  // Push 3 elements
  for (int i = 0; i < 3; i++) {
    void *result = samrena_vector_push(vec, &values[i]);
    assert(result != NULL);
  }

  assert(vec->size == 3);

  // Push 2 more elements (should trigger resize)
  for (int i = 3; i < 5; i++) {
    void *result = samrena_vector_push(vec, &values[i]);
    assert(result != NULL);
  }

  assert(vec->size == 5);
  assert(vec->capacity >= 5);

  // Verify all elements
  for (int i = 0; i < 5; i++) {
    int *element = (int *)((uint8_t *)vec->data + (i * vec->element_size));
    assert(*element == values[i]);
  }

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test popping elements from the vector
void test_vector_pop() {
  printf("Testing vector pop... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 5);

  int values[] = {10, 20, 30, 40, 50};

  // Push 5 elements
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  assert(vec->size == 5);

  // Pop elements and verify
  for (int i = 4; i >= 0; i--) {
    int *popped = (int *)samrena_vector_pop(vec);
    assert(popped != NULL);
    assert(*popped == values[i]);
    assert(vec->size == (uint64_t)i);
  }

  // Try popping from empty vector
  void *result = samrena_vector_pop(vec);
  assert(result == NULL);

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test vector resize
void test_vector_resize() {
  printf("Testing vector resize... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 5);

  int values[10];
  for (int i = 0; i < 10; i++) {
    values[i] = i * 10;
  }

  // Push 5 elements
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  uint64_t original_capacity = vec->capacity;

  // Manually resize vector to double capacity
  SamrenaVectorError result = samrena_vector_resize(vec, original_capacity * 2);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->capacity == original_capacity * 2);

  // Verify existing elements remain intact
  for (int i = 0; i < 5; i++) {
    int *element = (int *)((uint8_t *)vec->data + (i * vec->element_size));
    assert(*element == values[i]);
  }

  // Push more elements to fill expanded capacity
  for (int i = 5; i < 10; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  assert(vec->size == 10);

  // Verify all elements
  for (int i = 0; i < 10; i++) {
    int *element = (int *)((uint8_t *)vec->data + (i * vec->element_size));
    assert(*element == values[i]);
  }

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test with different data types
void test_different_types() {
  printf("Testing vector with different data types... ");

  Samrena *arena = samrena_create_default();

  // Test with char
  SamrenaVector *char_vec = samrena_vector_init(arena, sizeof(char), 5);
  char chars[] = {'a', 'b', 'c', 'd', 'e'};

  for (int i = 0; i < 5; i++) {
    samrena_vector_push(char_vec, &chars[i]);
  }

  for (int i = 0; i < 5; i++) {
    char *element = (char *)((uint8_t *)char_vec->data + (i * char_vec->element_size));
    assert(*element == chars[i]);
  }

  // Test with double
  SamrenaVector *double_vec = samrena_vector_init(arena, sizeof(double), 5);
  double doubles[] = {1.1, 2.2, 3.3, 4.4, 5.5};

  for (int i = 0; i < 5; i++) {
    samrena_vector_push(double_vec, &doubles[i]);
  }

  for (int i = 0; i < 5; i++) {
    double *element = (double *)((uint8_t *)double_vec->data + (i * double_vec->element_size));
    assert(*element == doubles[i]);
  }

  // Test with struct
  typedef struct {
    int id;
    char name[10];
    double value;
  } TestStruct;

  SamrenaVector *struct_vec = samrena_vector_init(arena, sizeof(TestStruct), 3);

  TestStruct structs[3];

  for (int i = 0; i < 3; i++) {
    structs[i].id = i + 1;
    snprintf(structs[i].name, 10, "Name%d", i + 1);
    structs[i].value = (i + 1) * 2.5;
    samrena_vector_push(struct_vec, &structs[i]);
  }

  for (int i = 0; i < 3; i++) {
    TestStruct *element =
        (TestStruct *)((uint8_t *)struct_vec->data + (i * struct_vec->element_size));
    assert(element->id == structs[i].id);
    assert(strcmp(element->name, structs[i].name) == 0);
    assert(element->value == structs[i].value);
  }

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test edge cases
void test_edge_cases() {
  printf("Testing vector edge cases... ");

  Samrena *arena = samrena_create_default();

  // Zero capacity
  SamrenaVector *vec_zero = samrena_vector_init(arena, sizeof(int), 0);
  assert(vec_zero != NULL);
  assert(vec_zero->capacity > 0); // Should use default capacity

  // Large number of elements
  int num_elements = 1000;
  SamrenaVector *vec_large = samrena_vector_init(arena, sizeof(int), num_elements);

  for (int i = 0; i < num_elements; i++) {
    samrena_vector_push(vec_large, &i);
  }

  assert(vec_large->size == (uint64_t)num_elements);

  // Verify some elements
  int *first = (int *)((uint8_t *)vec_large->data);
  int *middle =
      (int *)((uint8_t *)vec_large->data + ((num_elements / 2) * vec_large->element_size));
  int *last = (int *)((uint8_t *)vec_large->data + ((num_elements - 1) * vec_large->element_size));

  assert(*first == 0);
  assert(*middle == num_elements / 2);
  assert(*last == num_elements - 1);

  // Test small element size
  SamrenaVector *vec_small = samrena_vector_init(arena, 1, 10); // 1-byte elements
  uint8_t small_vals[] = {0xFF, 0xAA, 0x55, 0x01};

  for (int i = 0; i < 4; i++) {
    samrena_vector_push(vec_small, &small_vals[i]);
  }

  for (int i = 0; i < 4; i++) {
    uint8_t *element = (uint8_t *)vec_small->data + i;
    assert(*element == small_vals[i]);
  }

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test element access functions
void test_element_access() {
  printf("Testing element access functions... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 10);

  int values[] = {10, 20, 30, 40, 50};

  // Push 5 elements
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  // Test samrena_vector_get
  int retrieved;
  assert(samrena_vector_get(vec, 0, &retrieved) == SAMRENA_VECTOR_SUCCESS);
  assert(retrieved == 10);
  assert(samrena_vector_get(vec, 2, &retrieved) == SAMRENA_VECTOR_SUCCESS);
  assert(retrieved == 30);
  assert(samrena_vector_get(vec, 4, &retrieved) == SAMRENA_VECTOR_SUCCESS);
  assert(retrieved == 50);

  // Test out of bounds
  assert(samrena_vector_get(vec, 5, &retrieved) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
  assert(samrena_vector_get(vec, 10, &retrieved) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);

  // Test NULL parameters
  assert(samrena_vector_get(NULL, 0, &retrieved) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
  assert(samrena_vector_get(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);

  // Test samrena_vector_set
  int new_value = 99;
  assert(samrena_vector_set(vec, 2, &new_value) == SAMRENA_VECTOR_SUCCESS);
  assert(samrena_vector_get(vec, 2, &retrieved) == SAMRENA_VECTOR_SUCCESS);
  assert(retrieved == 99);

  // Test set out of bounds
  assert(samrena_vector_set(vec, 5, &new_value) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);

  // Test NULL parameters
  assert(samrena_vector_set(NULL, 0, &new_value) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
  assert(samrena_vector_set(vec, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test pointer access functions
void test_pointer_access() {
  printf("Testing pointer access functions... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 10);

  int values[] = {100, 200, 300, 400, 500};

  // Push 5 elements
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  // Test samrena_vector_at
  int *ptr = (int *)samrena_vector_at(vec, 0);
  assert(ptr != NULL);
  assert(*ptr == 100);

  ptr = (int *)samrena_vector_at(vec, 3);
  assert(ptr != NULL);
  assert(*ptr == 400);

  // Test out of bounds
  ptr = (int *)samrena_vector_at(vec, 5);
  assert(ptr == NULL);

  // Test NULL vector
  ptr = (int *)samrena_vector_at(NULL, 0);
  assert(ptr == NULL);

  // Test const version
  const int *const_ptr = (const int *)samrena_vector_at_const(vec, 1);
  assert(const_ptr != NULL);
  assert(*const_ptr == 200);

  const_ptr = (const int *)samrena_vector_at_const(vec, 5);
  assert(const_ptr == NULL);

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test element access convenience functions
void test_convenience_access() {
  printf("Testing element access functions... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 10);

  // Test with empty vector
  assert(samrena_vector_at(vec, 0) == NULL);
  assert(samrena_vector_at_const(vec, 0) == NULL);

  int values[] = {111, 222, 333, 444, 555};

  // Push 5 elements
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  // Test first element (front)
  int *front_ptr = (int *)samrena_vector_at(vec, 0);
  assert(front_ptr != NULL);
  assert(*front_ptr == 111);

  const int *const_front_ptr = (const int *)samrena_vector_at_const(vec, 0);
  assert(const_front_ptr != NULL);
  assert(*const_front_ptr == 111);

  // Test last element (back)
  int *back_ptr = (int *)samrena_vector_at(vec, 4);
  assert(back_ptr != NULL);
  assert(*back_ptr == 555);

  const int *const_back_ptr = (const int *)samrena_vector_at_const(vec, 4);
  assert(const_back_ptr != NULL);
  assert(*const_back_ptr == 555);

  // Test all elements using direct access
  for (int i = 0; i < 5; i++) {
    int *ptr = (int *)samrena_vector_at(vec, i);
    const int *const_ptr = (const int *)samrena_vector_at_const(vec, i);
    assert(ptr != NULL);
    assert(const_ptr != NULL);
    assert(*ptr == values[i]);
    assert(*const_ptr == values[i]);
  }

  // Test NULL vector and out of bounds
  assert(samrena_vector_at(NULL, 0) == NULL);
  assert(samrena_vector_at_const(NULL, 0) == NULL);
  assert(samrena_vector_at(vec, 10) == NULL);
  assert(samrena_vector_at_const(vec, 10) == NULL);

  samrena_destroy(arena);
  printf("PASSED\n");
}

// Test unsafe access functions
void test_unsafe_access() {
  printf("Testing unsafe access functions... ");

  Samrena *arena = samrena_create_default();
  SamrenaVector *vec = samrena_vector_init(arena, sizeof(int), 10);

  int values[] = {123, 456, 789, 321, 654};

  // Push 5 elements
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  // Test unchecked access
  int *unchecked_ptr = (int *)samrena_vector_at_unchecked(vec, 0);
  assert(*unchecked_ptr == 123);

  unchecked_ptr = (int *)samrena_vector_at_unchecked(vec, 2);
  assert(*unchecked_ptr == 789);

  unchecked_ptr = (int *)samrena_vector_at_unchecked(vec, 4);
  assert(*unchecked_ptr == 654);

  // Test const unchecked access
  const int *const_unchecked_ptr = (const int *)samrena_vector_at_unchecked_const(vec, 1);
  assert(*const_unchecked_ptr == 456);

  const_unchecked_ptr = (const int *)samrena_vector_at_unchecked_const(vec, 3);
  assert(*const_unchecked_ptr == 321);

  // Test macro access
  assert(SAMRENA_VECTOR_ELEM(vec, int, 0) == 123);
  assert(SAMRENA_VECTOR_ELEM(vec, int, 2) == 789);
  assert(SAMRENA_VECTOR_ELEM(vec, int, 4) == 654);

  // Modify through macro
  SAMRENA_VECTOR_ELEM(vec, int, 1) = 999;
  assert(SAMRENA_VECTOR_ELEM(vec, int, 1) == 999);

  samrena_destroy(arena);
  printf("PASSED\n");
}

int main() {
  printf("Starting samrena vector tests...\n\n");

  test_vector_init();
  test_vector_push();
  test_vector_pop();
  test_vector_resize();
  test_different_types();
  test_edge_cases();
  test_convenience_access();
  test_pointer_access();
  test_unsafe_access();

  printf("\nAll samrena vector tests passed!\n");
  return 0;
}
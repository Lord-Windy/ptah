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

SAMRENA_DECLARE_VECTOR(int);
SAMRENA_DECLARE_VECTOR(double);

typedef struct {
  int id;
  float value;
  char name[16];
} TestStruct;

SAMRENA_DECLARE_VECTOR(TestStruct);

void test_typesafe_int_vector() {
  printf("Testing type-safe int vector... ");

  SamrenaVector_int *vec = samrena_vector_int_init_owned(5);
  assert(vec != NULL);
  assert(vec->_vec != NULL);

  assert(samrena_vector_int_is_empty(vec) == true);
  assert(samrena_vector_int_size(vec) == 0);
  assert(samrena_vector_int_capacity(vec) == 5);

  int values[] = {10, 20, 30, 40, 50};

  for (int i = 0; i < 5; i++) {
    int *result = samrena_vector_int_push(vec, &values[i]);
    assert(result != NULL);
    assert(*result == values[i]);
    assert(samrena_vector_int_size(vec) == (size_t)(i + 1));
  }

  assert(samrena_vector_int_is_full(vec) == true);
  assert(samrena_vector_int_is_empty(vec) == false);

  for (int i = 0; i < 5; i++) {
    int *element = samrena_vector_int_at(vec, i);
    assert(element != NULL);
    assert(*element == values[i]);

    const int *const_element = samrena_vector_int_at_const(vec, i);
    assert(const_element != NULL);
    assert(*const_element == values[i]);

    int retrieved;
    assert(samrena_vector_int_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == values[i]);
  }

  for (int i = 4; i >= 0; i--) {
    int *popped = samrena_vector_int_pop(vec);
    assert(popped != NULL);
    assert(*popped == values[i]);
    assert(samrena_vector_int_size(vec) == (size_t)i);
  }

  assert(samrena_vector_int_is_empty(vec) == true);

  samrena_vector_int_destroy(vec);
  printf("PASSED\n");
}

void test_typesafe_double_vector() {
  printf("Testing type-safe double vector... ");

  SamrenaVector_double *vec = samrena_vector_double_init_owned(3);
  assert(vec != NULL);

  double values[] = {1.1, 2.2, 3.3, 4.4};

  for (int i = 0; i < 3; i++) {
    double *result = samrena_vector_double_push(vec, &values[i]);
    assert(result != NULL);
    assert(*result == values[i]);
  }

  assert(samrena_vector_double_is_full(vec) == true);

  double *growth_result = samrena_vector_double_push(vec, &values[3]);
  assert(growth_result != NULL);
  assert(*growth_result == values[3]);
  assert(samrena_vector_double_capacity(vec) > 3);

  for (int i = 0; i < 4; i++) {
    double retrieved;
    assert(samrena_vector_double_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == values[i]);
  }

  samrena_vector_double_clear(vec);
  assert(samrena_vector_double_is_empty(vec) == true);
  assert(samrena_vector_double_size(vec) == 0);

  samrena_vector_double_destroy(vec);
  printf("PASSED\n");
}

void test_typesafe_struct_vector() {
  printf("Testing type-safe struct vector... ");

  SamrenaVector_TestStruct *vec = samrena_vector_TestStruct_init_owned(2);
  assert(vec != NULL);

  TestStruct structs[] = {{1, 1.5f, "First"}, {2, 2.5f, "Second"}, {3, 3.5f, "Third"}};

  for (int i = 0; i < 3; i++) {
    TestStruct *result = samrena_vector_TestStruct_push(vec, &structs[i]);
    assert(result != NULL);
    assert(result->id == structs[i].id);
    assert(result->value == structs[i].value);
    assert(strcmp(result->name, structs[i].name) == 0);
  }

  assert(samrena_vector_TestStruct_size(vec) == 3);
  assert(samrena_vector_TestStruct_capacity(vec) > 2);

  for (int i = 0; i < 3; i++) {
    TestStruct *element = samrena_vector_TestStruct_at(vec, i);
    assert(element != NULL);
    assert(element->id == structs[i].id);
    assert(element->value == structs[i].value);
    assert(strcmp(element->name, structs[i].name) == 0);

    TestStruct retrieved;
    assert(samrena_vector_TestStruct_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved.id == structs[i].id);
    assert(retrieved.value == structs[i].value);
    assert(strcmp(retrieved.name, structs[i].name) == 0);
  }

  TestStruct new_struct = {99, 99.9f, "Updated"};
  assert(samrena_vector_TestStruct_set(vec, 1, &new_struct) == SAMRENA_VECTOR_SUCCESS);

  TestStruct *updated = samrena_vector_TestStruct_at(vec, 1);
  assert(updated != NULL);
  assert(updated->id == 99);
  assert(updated->value == 99.9f);
  assert(strcmp(updated->name, "Updated") == 0);

  samrena_vector_TestStruct_destroy(vec);
  printf("PASSED\n");
}

void test_typesafe_arena_allocation() {
  printf("Testing type-safe vector with arena allocation... ");

  Samrena *arena = samrena_create_default();
  assert(arena != NULL);

  SamrenaVector_int *vec = samrena_vector_int_init(arena, 5);
  assert(vec != NULL);

  int values[] = {100, 200, 300};

  for (int i = 0; i < 3; i++) {
    int *result = samrena_vector_int_push(vec, &values[i]);
    assert(result != NULL);
    assert(*result == values[i]);
  }

  assert(samrena_vector_int_size(vec) == 3);

  for (int i = 0; i < 3; i++) {
    int *element = samrena_vector_int_at(vec, i);
    assert(element != NULL);
    assert(*element == values[i]);
  }

  samrena_destroy(arena);
  printf("PASSED\n");
}

void test_typesafe_resize() {
  printf("Testing type-safe vector resize... ");

  SamrenaVector_int *vec = samrena_vector_int_init_owned(2);
  assert(vec != NULL);

  int values[] = {1, 2};
  for (int i = 0; i < 2; i++) {
    samrena_vector_int_push(vec, &values[i]);
  }

  assert(samrena_vector_int_capacity(vec) == 2);
  assert(samrena_vector_int_is_full(vec) == true);

  assert(samrena_vector_int_resize(vec, 10) == SAMRENA_VECTOR_SUCCESS);
  assert(samrena_vector_int_capacity(vec) == 10);
  assert(samrena_vector_int_is_full(vec) == false);
  assert(samrena_vector_int_size(vec) == 2);

  for (int i = 0; i < 2; i++) {
    int *element = samrena_vector_int_at(vec, i);
    assert(element != NULL);
    assert(*element == values[i]);
  }

  for (int i = 2; i < 10; i++) {
    int value = i * 10;
    int *result = samrena_vector_int_push(vec, &value);
    assert(result != NULL);
    assert(*result == value);
  }

  assert(samrena_vector_int_size(vec) == 10);
  assert(samrena_vector_int_is_full(vec) == true);

  samrena_vector_int_destroy(vec);
  printf("PASSED\n");
}

void test_typesafe_null_safety() {
  printf("Testing type-safe vector NULL safety... ");

  assert(samrena_vector_int_push(NULL, NULL) == NULL);
  assert(samrena_vector_int_pop(NULL) == NULL);
  assert(samrena_vector_int_at(NULL, 0) == NULL);
  assert(samrena_vector_int_at_const(NULL, 0) == NULL);
  assert(samrena_vector_int_get(NULL, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
  assert(samrena_vector_int_set(NULL, 0, NULL) == SAMRENA_VECTOR_ERROR_NULL_POINTER);
  assert(samrena_vector_int_size(NULL) == 0);
  assert(samrena_vector_int_capacity(NULL) == 0);
  assert(samrena_vector_int_is_empty(NULL) == true);
  assert(samrena_vector_int_is_full(NULL) == true);
  assert(samrena_vector_int_resize(NULL, 10) == SAMRENA_VECTOR_ERROR_NULL_POINTER);

  samrena_vector_int_clear(NULL);
  samrena_vector_int_destroy(NULL);

  printf("PASSED\n");
}

void test_typesafe_mixed_usage() {
  printf("Testing mixed type-safe and generic vector usage... ");

  SamrenaVector_int *typed_vec = samrena_vector_int_init_owned(5);
  SamrenaVector *generic_vec = samrena_vector_init_owned(sizeof(int), 5);

  assert(typed_vec != NULL);
  assert(generic_vec != NULL);

  int values[] = {10, 20, 30};

  for (int i = 0; i < 3; i++) {
    samrena_vector_int_push(typed_vec, &values[i]);
    samrena_vector_push(generic_vec, &values[i]);
  }

  assert(samrena_vector_int_size(typed_vec) == 3);
  assert(samrena_vector_size(generic_vec) == 3);

  for (int i = 0; i < 3; i++) {
    int *typed_elem = samrena_vector_int_at(typed_vec, i);
    int *generic_elem = (int *)samrena_vector_at(generic_vec, i);

    assert(typed_elem != NULL);
    assert(generic_elem != NULL);
    assert(*typed_elem == *generic_elem);
    assert(*typed_elem == values[i]);
  }

  samrena_vector_int_destroy(typed_vec);
  samrena_vector_destroy(generic_vec);
  printf("PASSED\n");
}

int main() {
  printf("Starting samrena type-safe vector tests...\n\n");

  test_typesafe_int_vector();
  test_typesafe_double_vector();
  test_typesafe_struct_vector();
  test_typesafe_arena_allocation();
  test_typesafe_resize();
  test_typesafe_null_safety();
  test_typesafe_mixed_usage();

  printf("\nAll samrena type-safe vector tests passed!\n");
  return 0;
}
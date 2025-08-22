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

void test_vector_resize_expand() {
  printf("Testing samrena_vector_resize expansion... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
  assert(vec != NULL);

  int values[] = {1, 2, 3, 4, 5};
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  assert(vec->size == 5);
  assert(vec->capacity == 5);

  // Expand capacity
  SamrenaVectorError result = samrena_vector_resize(vec, 20);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->capacity == 20);
  assert(vec->size == 5); // Size should remain unchanged

  // Verify existing data is preserved
  for (int i = 0; i < 5; i++) {
    int retrieved;
    assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == values[i]);
  }

  // Add more elements to use expanded capacity
  int new_values[] = {6, 7, 8, 9, 10};
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &new_values[i]);
  }

  assert(vec->size == 10);

  // Verify all data
  for (int i = 0; i < 5; i++) {
    int retrieved;
    assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == values[i]);
    assert(samrena_vector_get(vec, i + 5, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == new_values[i]);
  }

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_resize_shrink() {
  printf("Testing samrena_vector_resize shrinking... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 20);
  assert(vec != NULL);

  // Fill with data
  for (int i = 0; i < 15; i++) {
    samrena_vector_push(vec, &i);
  }

  assert(vec->size == 15);
  assert(vec->capacity == 20);

  // Shrink capacity but keep larger than size
  SamrenaVectorError result = samrena_vector_resize(vec, 18);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->capacity == 18);
  assert(vec->size == 15);

  // Verify data is preserved
  for (int i = 0; i < 15; i++) {
    int retrieved;
    assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == i);
  }

  // Shrink to exactly match size
  result = samrena_vector_resize(vec, 15);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->capacity == 15);
  assert(vec->size == 15);

  // Data should still be intact
  for (int i = 0; i < 15; i++) {
    int retrieved;
    assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == i);
  }

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_resize_error_cases() {
  printf("Testing samrena_vector_resize error cases... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  for (int i = 0; i < 8; i++) {
    samrena_vector_push(vec, &i);
  }

  // Test NULL vector
  assert(samrena_vector_resize(NULL, 20) == SAMRENA_VECTOR_ERROR_NULL_POINTER);

  // Test resizing smaller than current size (should succeed and truncate)
  assert(samrena_vector_resize(vec, 5) == SAMRENA_VECTOR_SUCCESS);
  assert(vec->size == 5); // Size should be truncated to new capacity

  // Test zero capacity (should succeed and clear everything)
  assert(samrena_vector_resize(vec, 0) == SAMRENA_VECTOR_SUCCESS);
  assert(vec->size == 0);
  assert(vec->capacity == 0);
  assert(vec->data == NULL);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_clear() {
  printf("Testing samrena_vector_clear... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  // Fill with data
  for (int i = 0; i < 8; i++) {
    samrena_vector_push(vec, &i);
  }

  assert(vec->size == 8);
  size_t original_capacity = vec->capacity;

  // Clear the vector
  samrena_vector_clear(vec);

  assert(vec->size == 0);
  assert(vec->capacity == original_capacity); // Capacity should remain unchanged

  // Should be able to add elements again
  int values[] = {100, 200, 300};
  for (int i = 0; i < 3; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  assert(vec->size == 3);
  for (int i = 0; i < 3; i++) {
    int retrieved;
    assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == values[i]);
  }

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_clear_null() {
  printf("Testing samrena_vector_clear with NULL... ");

  // Should handle NULL gracefully without crashing
  samrena_vector_clear(NULL);

  printf("PASSED\n");
}

void test_vector_truncate_success() {
  printf("Testing samrena_vector_truncate success cases... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  // Fill with data
  for (int i = 0; i < 8; i++) {
    samrena_vector_push(vec, &i);
  }

  assert(vec->size == 8);

  // Truncate to smaller size
  SamrenaVectorError result = samrena_vector_truncate(vec, 5);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->size == 5);

  // Verify remaining elements
  for (int i = 0; i < 5; i++) {
    int retrieved;
    assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == i);
  }

  // Should not be able to access truncated elements
  int dummy;
  assert(samrena_vector_get(vec, 5, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
  assert(samrena_vector_get(vec, 7, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);

  // Truncate to zero
  result = samrena_vector_truncate(vec, 0);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->size == 0);
  assert(samrena_vector_get(vec, 0, &dummy) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_truncate_error_cases() {
  printf("Testing samrena_vector_truncate error cases... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &i);
  }

  // Test NULL vector
  assert(samrena_vector_truncate(NULL, 3) == SAMRENA_VECTOR_ERROR_NULL_POINTER);

  // Test truncating to larger size (should fail)
  assert(samrena_vector_truncate(vec, 10) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);
  assert(samrena_vector_truncate(vec, 6) == SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS);

  // Vector should remain unchanged
  assert(vec->size == 5);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_reset_success() {
  printf("Testing samrena_vector_reset success cases... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  // Fill with data
  for (int i = 0; i < 8; i++) {
    samrena_vector_push(vec, &i);
  }

  assert(vec->size == 8);
  assert(vec->capacity == 10);

  // Reset with same capacity
  SamrenaVectorError result = samrena_vector_reset(vec, 10);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->size == 0);
  assert(vec->capacity == 10);

  // Should be able to add new data
  int values[] = {100, 200, 300};
  for (int i = 0; i < 3; i++) {
    samrena_vector_push(vec, &values[i]);
  }

  assert(vec->size == 3);
  for (int i = 0; i < 3; i++) {
    int retrieved;
    assert(samrena_vector_get(vec, i, &retrieved) == SAMRENA_VECTOR_SUCCESS);
    assert(retrieved == values[i]);
  }

  // Reset with different capacity
  result = samrena_vector_reset(vec, 20);
  assert(result == SAMRENA_VECTOR_SUCCESS);
  assert(vec->size == 0);
  assert(vec->capacity == 20);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_reset_error_cases() {
  printf("Testing samrena_vector_reset error cases... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  // Test NULL vector
  assert(samrena_vector_reset(NULL, 10) == SAMRENA_VECTOR_ERROR_NULL_POINTER);

  // Test zero capacity (should succeed)
  assert(samrena_vector_reset(vec, 0) == SAMRENA_VECTOR_SUCCESS);
  assert(vec->size == 0);
  assert(vec->capacity == 0);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_size() {
  printf("Testing samrena_vector_size... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  // Initially empty
  assert(samrena_vector_size(vec) == 0);

  // Add elements and check size
  for (int i = 0; i < 7; i++) {
    samrena_vector_push(vec, &i);
    assert(samrena_vector_size(vec) == (size_t)(i + 1));
  }

  // Remove elements and check size
  for (int i = 6; i >= 0; i--) {
    assert(samrena_vector_size(vec) == (size_t)(i + 1));
    samrena_vector_pop(vec);
  }

  assert(samrena_vector_size(vec) == 0);

  // Test with NULL vector
  assert(samrena_vector_size(NULL) == 0);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_capacity() {
  printf("Testing samrena_vector_capacity... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 15);
  assert(vec != NULL);

  assert(samrena_vector_capacity(vec) == 15);

  // Capacity should remain the same as we add elements (up to capacity)
  for (int i = 0; i < 10; i++) {
    samrena_vector_push(vec, &i);
    assert(samrena_vector_capacity(vec) == 15);
  }

  // Resize and check capacity
  samrena_vector_resize(vec, 25);
  assert(samrena_vector_capacity(vec) == 25);

  samrena_vector_resize(vec, 30);
  assert(samrena_vector_capacity(vec) == 30);

  // Test with NULL vector
  assert(samrena_vector_capacity(NULL) == 0);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_is_empty() {
  printf("Testing samrena_vector_is_empty... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  // Initially empty
  assert(samrena_vector_is_empty(vec) == true);

  // Add element - should not be empty
  int value = 42;
  samrena_vector_push(vec, &value);
  assert(samrena_vector_is_empty(vec) == false);

  // Add more elements - still not empty
  for (int i = 0; i < 5; i++) {
    samrena_vector_push(vec, &i);
    assert(samrena_vector_is_empty(vec) == false);
  }

  // Remove all elements - should be empty again
  while (samrena_vector_size(vec) > 0) {
    samrena_vector_pop(vec);
  }
  assert(samrena_vector_is_empty(vec) == true);

  // Clear and check
  for (int i = 0; i < 3; i++) {
    samrena_vector_push(vec, &i);
  }
  assert(samrena_vector_is_empty(vec) == false);

  samrena_vector_clear(vec);
  assert(samrena_vector_is_empty(vec) == true);

  // Test with NULL vector
  assert(samrena_vector_is_empty(NULL) == true);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_is_full() {
  printf("Testing samrena_vector_is_full... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 5);
  assert(vec != NULL);

  // Initially not full
  assert(samrena_vector_is_full(vec) == false);

  // Add elements until full
  for (int i = 0; i < 5; i++) {
    assert(samrena_vector_is_full(vec) == false);
    samrena_vector_push(vec, &i);
  }

  // Now should be full
  assert(samrena_vector_is_full(vec) == true);
  assert(samrena_vector_size(vec) == 5);
  assert(samrena_vector_capacity(vec) == 5);

  // Remove one element - should not be full anymore
  samrena_vector_pop(vec);
  assert(samrena_vector_is_full(vec) == false);

  // Add it back - should be full again
  int value = 99;
  samrena_vector_push(vec, &value);
  assert(samrena_vector_is_full(vec) == true);

  // Test with NULL vector
  assert(samrena_vector_is_full(NULL) == false);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_available() {
  printf("Testing samrena_vector_available... ");

  SamrenaVector *vec = samrena_vector_init_owned(sizeof(int), 10);
  assert(vec != NULL);

  // Initially all capacity available
  assert(samrena_vector_available(vec) == 10);

  // Add elements and check available space
  for (int i = 0; i < 7; i++) {
    assert(samrena_vector_available(vec) == (size_t)(10 - i));
    samrena_vector_push(vec, &i);
  }

  assert(samrena_vector_available(vec) == 3);

  // Fill to capacity
  for (int i = 7; i < 10; i++) {
    samrena_vector_push(vec, &i);
  }

  assert(samrena_vector_available(vec) == 0);
  assert(samrena_vector_is_full(vec) == true);

  // Remove elements and check available space
  samrena_vector_pop(vec);
  assert(samrena_vector_available(vec) == 1);

  samrena_vector_pop(vec);
  samrena_vector_pop(vec);
  assert(samrena_vector_available(vec) == 3);

  // Clear and check
  samrena_vector_clear(vec);
  assert(samrena_vector_available(vec) == 10);

  // Test with NULL vector
  assert(samrena_vector_available(NULL) == 0);

  samrena_vector_destroy(vec);
  printf("PASSED\n");
}

void test_vector_capacity_edge_cases() {
  printf("Testing vector capacity edge cases... ");

  // Test very small capacity
  SamrenaVector *small_vec = samrena_vector_init_owned(sizeof(int), 1);
  assert(small_vec != NULL);
  assert(samrena_vector_capacity(small_vec) == 1);
  assert(samrena_vector_available(small_vec) == 1);
  assert(samrena_vector_is_empty(small_vec) == true);
  assert(samrena_vector_is_full(small_vec) == false);

  int value = 42;
  samrena_vector_push(small_vec, &value);
  assert(samrena_vector_available(small_vec) == 0);
  assert(samrena_vector_is_full(small_vec) == true);

  samrena_vector_destroy(small_vec);

  // Test large capacity
  SamrenaVector *large_vec = samrena_vector_init_owned(sizeof(char), 100000);
  assert(large_vec != NULL);
  assert(samrena_vector_capacity(large_vec) == 100000);
  assert(samrena_vector_available(large_vec) == 100000);
  assert(samrena_vector_size(large_vec) == 0);

  samrena_vector_destroy(large_vec);

  printf("PASSED\n");
}

int main() {
  printf("Starting samrena vector capacity tests...\n\n");

  test_vector_resize_expand();
  test_vector_resize_shrink();
  test_vector_resize_error_cases();
  test_vector_clear();
  test_vector_clear_null();
  test_vector_truncate_success();
  test_vector_truncate_error_cases();
  test_vector_reset_success();
  test_vector_reset_error_cases();
  test_vector_size();
  test_vector_capacity();
  test_vector_is_empty();
  test_vector_is_full();
  test_vector_available();
  test_vector_capacity_edge_cases();

  printf("\nAll samrena vector capacity tests passed!\n");
  return 0;
}
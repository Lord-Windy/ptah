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

#include <assert.h>
#include <samdata/samset.h>
#include <samrena.h>
#include <stdio.h>
#include <string.h>

static void test_samset_empty_set_operations(void) {
  printf("Testing operations on empty set...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 16, arena);
  assert(samset != NULL);

  assert(samset_size(samset) == 0);
  assert(samset_is_empty(samset) == true);

  int value = 42;
  assert(samset_contains(samset, &value) == false);
  assert(samset_remove(samset, &value) == false);
  assert(samset_get_last_error(samset) == SAMSET_ERROR_ELEMENT_NOT_FOUND);

  samset_clear(samset);
  assert(samset_size(samset) == 0);
  assert(samset_is_empty(samset) == true);

  SamSetStats stats = samset_get_stats(samset);
  assert(stats.total_operations >= 1);
  assert(stats.total_collisions == 0);
  assert(stats.max_chain_length == 0);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Empty set operations test passed\n");
}

static void test_samset_single_element_operations(void) {
  printf("Testing operations with single element...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 16, arena);
  assert(samset != NULL);

  int value = 100;

  bool result = samset_add(samset, &value);
  assert(result == true);
  assert(samset_size(samset) == 1);
  assert(samset_is_empty(samset) == false);
  assert(samset_contains(samset, &value) == true);

  result = samset_add(samset, &value);
  assert(result == false);
  assert(samset_size(samset) == 1);
  assert(samset_get_last_error(samset) == SAMSET_ERROR_ELEMENT_EXISTS);

  result = samset_remove(samset, &value);
  assert(result == true);
  assert(samset_size(samset) == 0);
  assert(samset_is_empty(samset) == true);
  assert(samset_contains(samset, &value) == false);

  result = samset_remove(samset, &value);
  assert(result == false);
  assert(samset_get_last_error(samset) == SAMSET_ERROR_ELEMENT_NOT_FOUND);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Single element operations test passed\n");
}

static void test_samset_zero_element_size(void) {
  printf("Testing creation with zero element size...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(0, 16, arena);
  assert(samset == NULL);

  samrena_destroy(arena);
  printf("✓ Zero element size test passed\n");
}

static void test_samset_minimum_capacity(void) {
  printf("Testing creation with very small capacity...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 1, arena);
  assert(samset != NULL);

  int values[] = {1, 2, 3, 4, 5};
  for (int i = 0; i < 5; i++) {
    bool result = samset_add(samset, &values[i]);
    assert(result == true);
    assert(samset_contains(samset, &values[i]) == true);
  }

  assert(samset_size(samset) == 5);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Minimum capacity test passed\n");
}

static void test_samset_duplicate_bytes(void) {
  printf("Testing elements with identical byte patterns...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  typedef struct {
    int a;
    int b;
  } TestStruct;

  SamSet *samset = samset_create(sizeof(TestStruct), 16, arena);
  assert(samset != NULL);

  TestStruct s1 = {1, 2};
  TestStruct s2 = {1, 2};
  TestStruct s3 = {2, 1};

  bool result = samset_add(samset, &s1);
  assert(result == true);
  assert(samset_size(samset) == 1);

  result = samset_add(samset, &s2);
  assert(result == false);
  assert(samset_size(samset) == 1);
  assert(samset_get_last_error(samset) == SAMSET_ERROR_ELEMENT_EXISTS);

  result = samset_add(samset, &s3);
  assert(result == true);
  assert(samset_size(samset) == 2);

  assert(samset_contains(samset, &s1) == true);
  assert(samset_contains(samset, &s2) == true);
  assert(samset_contains(samset, &s3) == true);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Duplicate bytes test passed\n");
}

static void test_samset_large_element_size(void) {
  printf("Testing with large element size...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  typedef struct {
    char data[1024];
  } LargeStruct;

  SamSet *samset = samset_create(sizeof(LargeStruct), 16, arena);
  assert(samset != NULL);

  LargeStruct large1, large2;
  memset(&large1, 0xAA, sizeof(LargeStruct));
  memset(&large2, 0xBB, sizeof(LargeStruct));

  bool result = samset_add(samset, &large1);
  assert(result == true);
  assert(samset_contains(samset, &large1) == true);
  assert(samset_contains(samset, &large2) == false);

  result = samset_add(samset, &large2);
  assert(result == true);
  assert(samset_size(samset) == 2);

  assert(samset_contains(samset, &large1) == true);
  assert(samset_contains(samset, &large2) == true);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Large element size test passed\n");
}

static bool always_equal(const void *a, const void *b, size_t size) {
  (void)a;
  (void)b;
  (void)size;
  return true;
}

static uint32_t same_hash(const void *element, size_t size) {
  (void)element;
  (void)size;
  return 42;
}

static void test_samset_custom_equality_function(void) {
  printf("Testing custom equality function...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create_custom(sizeof(int), 16, arena, same_hash, always_equal);
  assert(samset != NULL);

  int value1 = 10;
  int value2 = 20;

  bool result = samset_add(samset, &value1);
  assert(result == true);
  assert(samset_size(samset) == 1);

  result = samset_add(samset, &value2);
  assert(result == false);
  assert(samset_get_last_error(samset) == SAMSET_ERROR_ELEMENT_EXISTS);
  assert(samset_size(samset) == 1);

  assert(samset_contains(samset, &value1) == true);
  assert(samset_contains(samset, &value2) == true);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Custom equality function test passed\n");
}

static void test_samset_operations_after_clear(void) {
  printf("Testing operations after clear...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 16, arena);
  assert(samset != NULL);

  int values[] = {1, 2, 3, 4, 5};
  for (int i = 0; i < 5; i++) {
    samset_add(samset, &values[i]);
  }

  assert(samset_size(samset) == 5);

  samset_clear(samset);
  assert(samset_size(samset) == 0);
  assert(samset_is_empty(samset) == true);

  for (int i = 0; i < 5; i++) {
    assert(samset_contains(samset, &values[i]) == false);
  }

  for (int i = 0; i < 5; i++) {
    bool result = samset_add(samset, &values[i]);
    assert(result == true);
    assert(samset_contains(samset, &values[i]) == true);
  }

  assert(samset_size(samset) == 5);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Operations after clear test passed\n");
}

static SamSetError callback_error = SAMSET_ERROR_NONE;
static const char *callback_message = NULL;

static void error_callback(SamSetError error, const char *message, void *user_data) {
  (void)user_data;
  callback_error = error;
  callback_message = message;
}

static void test_samset_error_callback(void) {
  printf("Testing error callback functionality...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 16, arena);
  assert(samset != NULL);

  samset_set_error_callback(samset, error_callback, NULL);

  int value = 42;
  samset_add(samset, &value);

  callback_error = SAMSET_ERROR_NONE;
  callback_message = NULL;

  bool result = samset_add(samset, &value);
  assert(result == false);
  assert(callback_error == SAMSET_ERROR_ELEMENT_EXISTS);
  assert(callback_message != NULL);
  assert(strcmp(callback_message, "Element already exists") == 0);

  callback_error = SAMSET_ERROR_NONE;
  callback_message = NULL;

  int non_existent = 999;
  result = samset_remove(samset, &non_existent);
  assert(result == false);
  assert(callback_error == SAMSET_ERROR_ELEMENT_NOT_FOUND);
  assert(callback_message != NULL);
  assert(strcmp(callback_message, "Element not found") == 0);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ Error callback test passed\n");
}

int main(void) {
  printf("=== SamSet Edge Cases Tests ===\n");

  test_samset_empty_set_operations();
  test_samset_single_element_operations();
  test_samset_zero_element_size();
  test_samset_minimum_capacity();
  test_samset_duplicate_bytes();
  test_samset_large_element_size();
  test_samset_custom_equality_function();
  test_samset_operations_after_clear();
  test_samset_error_callback();

  printf("\n✅ All SamSet edge cases tests passed!\n");
  return 0;
}
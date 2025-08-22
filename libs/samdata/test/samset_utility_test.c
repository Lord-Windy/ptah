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

static void test_samset_copy(void) {
  printf("Testing SamSet copy...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena1 = samrena_create(&config);
  Samrena *arena2 = samrena_create(&config);
  assert(arena1 != NULL);
  assert(arena2 != NULL);

  SamSet *original = samset_create(sizeof(int), 16, arena1);
  assert(original != NULL);

  int values[] = {10, 20, 30, 40, 50};
  for (int i = 0; i < 5; i++) {
    assert(samset_add(original, &values[i]) == true);
  }
  assert(samset_size(original) == 5);

  SamSet *copy = samset_copy(original, arena2);
  assert(copy != NULL);
  assert(samset_size(copy) == samset_size(original));

  for (int i = 0; i < 5; i++) {
    assert(samset_contains(copy, &values[i]) == true);
  }

  int new_value = 60;
  assert(samset_add(copy, &new_value) == true);
  assert(samset_size(copy) == 6);
  assert(samset_size(original) == 5);
  assert(samset_contains(copy, &new_value) == true);
  assert(samset_contains(original, &new_value) == false);

  samset_destroy(original);
  samset_destroy(copy);
  samrena_destroy(arena1);
  samrena_destroy(arena2);
  printf("✓ SamSet copy test passed\n");
}

static void test_samset_copy_empty(void) {
  printf("Testing SamSet copy empty set...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena1 = samrena_create(&config);
  Samrena *arena2 = samrena_create(&config);
  assert(arena1 != NULL);
  assert(arena2 != NULL);

  SamSet *original = samset_create(sizeof(int), 16, arena1);
  assert(original != NULL);
  assert(samset_is_empty(original) == true);

  SamSet *copy = samset_copy(original, arena2);
  assert(copy != NULL);
  assert(samset_is_empty(copy) == true);
  assert(samset_size(copy) == 0);

  samset_destroy(original);
  samset_destroy(copy);
  samrena_destroy(arena1);
  samrena_destroy(arena2);
  printf("✓ SamSet copy empty test passed\n");
}

static void test_samset_to_array(void) {
  printf("Testing SamSet to array conversion...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 16, arena);
  assert(samset != NULL);

  int values[] = {10, 20, 30, 40, 50};
  for (int i = 0; i < 5; i++) {
    assert(samset_add(samset, &values[i]) == true);
  }

  int result_array[10];
  size_t count = samset_to_array(samset, result_array, 10);
  assert(count == 5);

  for (size_t i = 0; i < count; i++) {
    bool found = false;
    for (int j = 0; j < 5; j++) {
      if (result_array[i] == values[j]) {
        found = true;
        break;
      }
    }
    assert(found == true);
  }

  int small_array[3];
  count = samset_to_array(samset, small_array, 3);
  assert(count == 3);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ SamSet to array test passed\n");
}

static void test_samset_to_array_empty(void) {
  printf("Testing SamSet to array empty set...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 16, arena);
  assert(samset != NULL);

  int result_array[10];
  size_t count = samset_to_array(samset, result_array, 10);
  assert(count == 0);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ SamSet to array empty test passed\n");
}

static void test_samset_from_array(void) {
  printf("Testing SamSet from array creation...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  int values[] = {10, 20, 30, 40, 50, 20, 30};
  SamSet *samset = samset_from_array(values, 7, sizeof(int), arena);
  assert(samset != NULL);

  assert(samset_size(samset) == 5);

  for (int i = 0; i < 5; i++) {
    assert(samset_contains(samset, &values[i]) == true);
  }

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ SamSet from array test passed\n");
}

static void test_samset_from_array_empty(void) {
  printf("Testing SamSet from empty array...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  int values[] = {10, 20, 30};
  SamSet *samset = samset_from_array(values, 0, sizeof(int), arena);
  assert(samset == NULL);

  samrena_destroy(arena);
  printf("✓ SamSet from empty array test passed\n");
}

static void test_samset_roundtrip(void) {
  printf("Testing SamSet array roundtrip...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena1 = samrena_create(&config);
  Samrena *arena2 = samrena_create(&config);
  assert(arena1 != NULL);
  assert(arena2 != NULL);

  int original_values[] = {1, 5, 10, 15, 20, 25, 30};
  SamSet *original = samset_from_array(original_values, 7, sizeof(int), arena1);
  assert(original != NULL);
  assert(samset_size(original) == 7);

  int extracted[10];
  size_t count = samset_to_array(original, extracted, 10);
  assert(count == 7);

  SamSet *roundtrip = samset_from_array(extracted, count, sizeof(int), arena2);
  assert(roundtrip != NULL);
  assert(samset_size(roundtrip) == 7);

  for (int i = 0; i < 7; i++) {
    assert(samset_contains(original, &original_values[i]) == true);
    assert(samset_contains(roundtrip, &original_values[i]) == true);
  }

  samset_destroy(original);
  samset_destroy(roundtrip);
  samrena_destroy(arena1);
  samrena_destroy(arena2);
  printf("✓ SamSet roundtrip test passed\n");
}

static void test_samset_phase3_error_conditions(void) {
  printf("Testing SamSet phase 3 error conditions...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  SamSet *samset = samset_create(sizeof(int), 16, arena);
  assert(samset != NULL);

  assert(samset_copy(NULL, arena) == NULL);
  assert(samset_copy(samset, NULL) == NULL);

  assert(samset_to_array(NULL, NULL, 0) == 0);
  assert(samset_to_array(samset, NULL, 10) == 0);

  assert(samset_from_array(NULL, 10, sizeof(int), arena) == NULL);
  assert(samset_from_array("test", 10, 0, arena) == NULL);
  assert(samset_from_array("test", 10, sizeof(int), NULL) == NULL);

  samset_destroy(samset);
  samrena_destroy(arena);
  printf("✓ SamSet phase 3 error conditions test passed\n");
}

int main(void) {
  printf("Running SamSet Phase 3 Tests...\n\n");

  test_samset_copy();
  test_samset_copy_empty();
  test_samset_to_array();
  test_samset_to_array_empty();
  test_samset_from_array();
  test_samset_from_array_empty();
  test_samset_roundtrip();
  test_samset_phase3_error_conditions();

  printf("\n✓ All SamSet Phase 3 tests passed!\n");
  return 0;
}
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

static void test_samset_int_set_operations(void) {
  printf("Testing int_set type-safe operations...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  int_set_samset *set = int_set_create(16, arena);
  assert(set != NULL);
  assert(set->base != NULL);

  assert(int_set_size(set) == 0);
  assert(int_set_is_empty(set) == true);

  bool result = int_set_add(set, 10);
  assert(result == true);
  assert(int_set_size(set) == 1);
  assert(int_set_is_empty(set) == false);
  assert(int_set_contains(set, 10) == true);
  assert(int_set_contains(set, 20) == false);

  result = int_set_add(set, 20);
  assert(result == true);
  result = int_set_add(set, 30);
  assert(result == true);
  result = int_set_add(set, 40);
  assert(result == true);

  assert(int_set_size(set) == 4);

  result = int_set_add(set, 20);
  assert(result == false);
  assert(int_set_size(set) == 4);

  result = int_set_remove(set, 30);
  assert(result == true);
  assert(int_set_size(set) == 3);
  assert(int_set_contains(set, 30) == false);
  assert(int_set_contains(set, 20) == true);

  int_set_clear(set);
  assert(int_set_size(set) == 0);
  assert(int_set_is_empty(set) == true);

  int_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ int_set operations test passed\n");
}

static void test_samset_long_set_operations(void) {
  printf("Testing long_set type-safe operations...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  long_set_samset *set = long_set_create(16, arena);
  assert(set != NULL);

  long values[] = {1000000L, 2000000L, 3000000L, 4000000L, 5000000L};
  int num_values = sizeof(values) / sizeof(values[0]);

  for (int i = 0; i < num_values; i++) {
    bool result = long_set_add(set, values[i]);
    assert(result == true);
    assert(long_set_contains(set, values[i]) == true);
  }

  assert(long_set_size(set) == (size_t)num_values);

  for (int i = 0; i < num_values; i++) {
    assert(long_set_contains(set, values[i]) == true);
  }

  long_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ long_set operations test passed\n");
}

static void test_samset_ptr_set_operations(void) {
  printf("Testing ptr_set type-safe operations...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  ptr_set_samset *set = ptr_set_create(16, arena);
  assert(set != NULL);

  int a = 1, b = 2, c = 3, d = 4;
  void *ptrs[] = {&a, &b, &c, &d};
  int num_ptrs = sizeof(ptrs) / sizeof(ptrs[0]);

  for (int i = 0; i < num_ptrs; i++) {
    bool result = ptr_set_add(set, ptrs[i]);
    assert(result == true);
    assert(ptr_set_contains(set, ptrs[i]) == true);
  }

  assert(ptr_set_size(set) == (size_t)num_ptrs);

  void *null_ptr = NULL;
  bool result = ptr_set_add(set, null_ptr);
  assert(result == true);
  assert(ptr_set_contains(set, null_ptr) == true);
  assert(ptr_set_size(set) == (size_t)num_ptrs + 1);

  result = ptr_set_remove(set, null_ptr);
  assert(result == true);
  assert(ptr_set_contains(set, null_ptr) == false);
  assert(ptr_set_size(set) == (size_t)num_ptrs);

  ptr_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ ptr_set operations test passed\n");
}

static void test_samset_uint_set_operations(void) {
  printf("Testing uint_set type-safe operations...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  uint_set_samset *set = uint_set_create(16, arena);
  assert(set != NULL);

  unsigned int values[] = {0U, 1U, 100U, 1000U, 10000U, UINT32_MAX};
  int num_values = sizeof(values) / sizeof(values[0]);

  for (int i = 0; i < num_values; i++) {
    bool result = uint_set_add(set, values[i]);
    assert(result == true);
    assert(uint_set_contains(set, values[i]) == true);
  }

  assert(uint_set_size(set) == (size_t)num_values);

  bool result = uint_set_remove(set, UINT32_MAX);
  assert(result == true);
  assert(uint_set_contains(set, UINT32_MAX) == false);
  assert(uint_set_size(set) == (size_t)num_values - 1);

  uint_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ uint_set operations test passed\n");
}

static void test_samset_ulong_set_operations(void) {
  printf("Testing ulong_set type-safe operations...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  ulong_set_samset *set = ulong_set_create(16, arena);
  assert(set != NULL);

  unsigned long values[] = {0UL, 1UL, 1000000UL, 2000000UL, 3000000UL};
  int num_values = sizeof(values) / sizeof(values[0]);

  for (int i = 0; i < num_values; i++) {
    bool result = ulong_set_add(set, values[i]);
    assert(result == true);
    assert(ulong_set_contains(set, values[i]) == true);
  }

  assert(ulong_set_size(set) == (size_t)num_values);

  ulong_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ ulong_set operations test passed\n");
}

typedef struct {
  int callback_count;
  int last_value;
} IteratorData;

static void int_iterator_callback(int element, void *user_data) {
  IteratorData *data = (IteratorData *)user_data;
  data->callback_count++;
  data->last_value = element;
}

static void test_samset_foreach_iterator(void) {
  printf("Testing foreach iterator functionality...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  int_set_samset *set = int_set_create(16, arena);
  assert(set != NULL);

  int values[] = {10, 20, 30, 40, 50};
  int num_values = sizeof(values) / sizeof(values[0]);

  for (int i = 0; i < num_values; i++) {
    int_set_add(set, values[i]);
  }

  IteratorData data = {0, 0};
  int_set_foreach(set, int_iterator_callback, &data);

  assert(data.callback_count == num_values);

  int_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ foreach iterator test passed\n");
}

static void test_samset_null_parameter_handling(void) {
  printf("Testing null parameter handling in type-safe wrappers...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  int_set_samset *null_set = NULL;

  assert(int_set_add(null_set, 10) == false);
  assert(int_set_remove(null_set, 10) == false);
  assert(int_set_contains(null_set, 10) == false);
  assert(int_set_size(null_set) == 0);
  assert(int_set_is_empty(null_set) == true);

  int_set_clear(null_set);
  int_set_destroy(null_set);

  int_set_foreach(null_set, int_iterator_callback, NULL);

  samrena_destroy(arena);
  printf("✓ Null parameter handling test passed\n");
}

SAMSET_DEFINE_TYPED(custom_int_set, int)

static void test_samset_custom_type_definition(void) {
  printf("Testing custom type definition macro...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  custom_int_set_samset *set = custom_int_set_create(16, arena);
  assert(set != NULL);

  bool result = custom_int_set_add(set, 100);
  assert(result == true);
  assert(custom_int_set_contains(set, 100) == true);
  assert(custom_int_set_size(set) == 1);
  assert(custom_int_set_is_empty(set) == false);

  result = custom_int_set_remove(set, 100);
  assert(result == true);
  assert(custom_int_set_contains(set, 100) == false);
  assert(custom_int_set_size(set) == 0);
  assert(custom_int_set_is_empty(set) == true);

  custom_int_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ Custom type definition test passed\n");
}

typedef struct {
  int x, y;
} Point;

SAMSET_DEFINE_TYPED(point_set, Point)

static void test_samset_struct_type_definition(void) {
  printf("Testing struct type definition...\n");

  SamrenaConfig config = samrena_default_config();
  Samrena *arena = samrena_create(&config);
  assert(arena != NULL);

  point_set_samset *set = point_set_create(16, arena);
  assert(set != NULL);

  Point p1 = {10, 20};
  Point p2 = {30, 40};
  Point p3 = {10, 20};

  bool result = point_set_add(set, p1);
  assert(result == true);
  assert(point_set_contains(set, p1) == true);
  assert(point_set_contains(set, p3) == true);
  assert(point_set_size(set) == 1);

  result = point_set_add(set, p2);
  assert(result == true);
  assert(point_set_contains(set, p2) == true);
  assert(point_set_size(set) == 2);

  result = point_set_add(set, p3);
  assert(result == false);
  assert(point_set_size(set) == 2);

  point_set_destroy(set);
  samrena_destroy(arena);
  printf("✓ Struct type definition test passed\n");
}

int main(void) {
  printf("=== SamSet Type-Safe Tests ===\n");

  test_samset_int_set_operations();
  test_samset_long_set_operations();
  test_samset_ptr_set_operations();
  test_samset_uint_set_operations();
  test_samset_ulong_set_operations();
  test_samset_foreach_iterator();
  test_samset_null_parameter_handling();
  test_samset_custom_type_definition();
  test_samset_struct_type_definition();

  printf("\n✅ All SamSet type-safe tests passed!\n");
  return 0;
}
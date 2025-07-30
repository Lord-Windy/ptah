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
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

char *global_dictionary[50] = {
    "ability",  "about",    "above",    "accept",    "according", "account",
    "across",   "action",   "activity", "actually",  "address",   "administration",
    "admit",    "adult",    "affect",   "after",     "again",     "against",
    "age",      "agency",   "agent",    "agreement", "ahead",     "air",
    "all",      "allow",    "almost",   "alone",     "along",     "already",
    "also",     "although", "always",   "American",  "among",     "amount",
    "analysis", "and",      "animal",   "another",   "answer",    "any",
    "anyone",   "anything", "appear",   "apply",     "approach",  "area",
    "argue",    "arm"};

void print_samrena(Samrena *samrena) {

  printf("Arena allocated: %lu\n", samrena_allocated(samrena));
  printf("Arena capacity: %lu\n", samrena_capacity(samrena));
}

void create_new_arena() {
  Samrena *samrena = samrena_create_default();

  print_samrena(samrena);

  int32_t *data = samrena_push_zero(samrena, 400 * sizeof(int32_t));

  printf("Address of Data %p\n", (void *)data);
  printf("Address of Samrena? %p\n", (void *)samrena);
  printf("Address of Samrena + sizeof %p\n", (void *)samrena + sizeof(Samrena));

  assert(data != 0);
  // Data should be properly aligned, not necessarily at samrena + sizeof(Samrena)
  assert((uintptr_t)data >= (uintptr_t)samrena + sizeof(Samrena));
  // Verify data is properly aligned for int32_t
  assert(((uintptr_t)data % sizeof(int32_t)) == 0);

  print_samrena(samrena);
  samrena_destroy(samrena);
}

void create_multiple_arrays() {
  Samrena *samrena = samrena_create_default();

  int32_t *data_holder[10];

  for (int i = 0; i < 10; i++) {
    data_holder[i] = samrena_push_zero(samrena, 30 * sizeof(int32_t));
    for (int j = 0; j < 30; j++) {
      data_holder[i][j] = i * 30 + j;
      printf("data_holder[%d][%d] = %d\n", i, j, data_holder[i][j]);
    }
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 30; j++) {
      printf("data_holder[%d][%d] = %d\n", i, j, data_holder[i][j]);
      assert(data_holder[i][j] == i * 30 + j);
    }
  }

  print_samrena(samrena);
  samrena_destroy(samrena);
}

void create_multiple_strings() {
  Samrena *samrena = samrena_create_default();

  char **string_holder[10];

  for (int i = 0; i < 10; i++) {
    string_holder[i] = samrena_push(samrena, sizeof(char *) * 50);
    for (int j = 0; j < 50; j++) {
      char *new_string =
          samrena_push_zero(samrena, sizeof(char) * strlen(global_dictionary[j]) + 1);
      strcpy(new_string, global_dictionary[j]);
      string_holder[i][j] = new_string;
      printf("string_holder[%d][%d] = %s\n", i, j, string_holder[i][j]);
    }
  }

  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 50; j++) {
      printf("string_holder[%d][%d] = %s\n", i, j, string_holder[i][j]);
      assert(strcmp(string_holder[i][j], global_dictionary[j]) == 0);
    }
  }

  print_samrena(samrena);
  samrena_destroy(samrena);
}

// Test default allocation
void test_zero_page_allocation() {
  printf("\n--- Testing default allocation ---\n");
  Samrena *samrena = samrena_create_default();

  // Default allocation should return a valid arena
  assert(samrena != NULL);
  printf("Default allocation correctly returns valid arena\n");
  printf("Arena capacity: %lu bytes\n", samrena_capacity(samrena));
  printf("Arena allocated: %lu bytes\n", samrena_allocated(samrena));
  
  samrena_destroy(samrena);
}

// Test allocation at capacity boundary
void test_capacity_boundary() {
  printf("\n--- Testing capacity boundary ---\n");
  Samrena *samrena = samrena_create_default(); // Single page
  print_samrena(samrena);

  uint64_t remaining = samrena_capacity(samrena) - samrena_allocated(samrena);
  printf("Remaining space: %lu bytes\n", remaining);

  // Allocate exactly the remaining space
  void *data1 = samrena_push(samrena, remaining);
  assert(data1 != 0);
  printf("Allocated exact remaining space: %p\n", data1);
  print_samrena(samrena);

  // Try to allocate 1 more byte (should succeed by growing)
  void *data2 = samrena_push(samrena, 1);
  assert(data2 != 0);
  printf("Allocation beyond original capacity: %p\n", data2);
  printf("Arena grew - new capacity: %lu\n", samrena_capacity(samrena));

  samrena_destroy(samrena);
}

// Test alignment issues with different data types
void test_data_alignment() {
  printf("\n--- Testing data alignment ---\n");
  Samrena *samrena = samrena_create_default();

  // Push data of different types to test alignment
  char *c = samrena_push(samrena, sizeof(char));
  printf("Char address: %p\n", (void *)c);

  // This may cause alignment issues on some platforms
  int64_t *i64 = samrena_push(samrena, sizeof(int64_t));
  printf("Int64 address: %p\n", (void *)i64);
  assert(i64 != 0);

  // Try to write to the allocated memory
  *c = 'A';
  *i64 = 0x1234567890ABCDEF;

  printf("Char value: %c\n", *c);
  printf("Int64 value: %lx\n", *i64);

  samrena_destroy(samrena);
}

// Test extremely large allocation
void test_large_allocation() {
  printf("\n--- Testing large allocation ---\n");
  // Large but should be valid on most systems
  Samrena *samrena = samrena_create_default(); // 4MB

  print_samrena(samrena);

  // Allocate a large chunk but still within capacity
  void *large_data = samrena_push_zero(samrena, 1024 * 1024 * 2); // 2MB
  assert(large_data != 0);

  print_samrena(samrena);

  // Try to access the memory to ensure it's usable
  uint8_t *bytes = (uint8_t *)large_data;
  bytes[0] = 42;
  bytes[1024 * 1024] = 43;         // Middle of allocation
  bytes[2 * 1024 * 1024 - 1] = 44; // End of allocation

  assert(bytes[0] == 42);
  assert(bytes[1024 * 1024] == 43);
  assert(bytes[2 * 1024 * 1024 - 1] == 44);

  samrena_destroy(samrena);
}

// Test minimal size allocation
void test_minimal_allocation() {
  printf("\n--- Testing minimal allocations ---\n");
  Samrena *samrena = samrena_create_default();

  // Test with many small allocations
  void *pointers[1000];
  int count = 0;

  for (int i = 0; i < 1000; i++) {
    pointers[i] = samrena_push(samrena, 1); // Single byte allocations
    if (pointers[i] == 0) {
      break;
    }
    count++;

    // Write to each allocated byte to ensure it's valid
    *((uint8_t *)pointers[i]) = i % 256;
  }

  printf("Successfully allocated %d individual bytes\n", count);

  // Verify data
  for (int i = 0; i < count; i++) {
    assert(*((uint8_t *)pointers[i]) == i % 256);
  }

  print_samrena(samrena);
  samrena_destroy(samrena);
}

/*
void test_resize_array_basic() {
  printf("\n--- Testing samrena_resize_array basic functionality ---\n");
  Samrena *samrena = samrena_create_default();

  int32_t *original = samrena_push(samrena, 5 * sizeof(int32_t));
  assert(original != 0);

  for (int i = 0; i < 5; i++) {
    original[i] = i * 10;
  }

  printf("Original array created with 5 elements\n");

  int32_t *resized =
      samrena_resize_array(samrena, original, 5 * sizeof(int32_t), 8 * sizeof(int32_t));
  assert(resized != 0);

  for (int i = 0; i < 5; i++) {
    assert(resized[i] == i * 10);
    printf("resized[%d] = %d (preserved)\n", i, resized[i]);
  }

  for (int i = 5; i < 8; i++) {
    resized[i] = i * 10;
    printf("resized[%d] = %d (new)\n", i, resized[i]);
  }

  samrena_destroy(samrena);
}
*/

/*
void test_resize_array_shrink() {
  printf("\n--- Testing samrena_resize_array shrinking ---\n");
  Samrena *samrena = samrena_create_default();

  int32_t *original = samrena_push(samrena, 10 * sizeof(int32_t));
  assert(original != 0);

  for (int i = 0; i < 10; i++) {
    original[i] = i * 5;
  }

  printf("Original array created with 10 elements\n");

  int32_t *resized =
      samrena_resize_array(samrena, original, 10 * sizeof(int32_t), 4 * sizeof(int32_t));
  assert(resized != 0);

  for (int i = 0; i < 4; i++) {
    assert(resized[i] == i * 5);
    printf("resized[%d] = %d (preserved)\n", i, resized[i]);
  }

  samrena_destroy(samrena);
}
*/

/*
void test_resize_array_zero_sizes() {
  printf("\n--- Testing samrena_resize_array with zero sizes ---\n");
  Samrena *samrena = samrena_create_default();

  void *resized1 = samrena_resize_array(samrena, 0, 0, 100);
  assert(resized1 != 0);
  printf("Resize from null pointer succeeded\n");

  int32_t *original = samrena_push(samrena, 5 * sizeof(int32_t));
  assert(original != 0);
  original[0] = 42;

  void *resized2 = samrena_resize_array(samrena, original, 5 * sizeof(int32_t), 0);
  assert(resized2 != 0);
  printf("Resize to zero size succeeded\n");

  samrena_destroy(samrena);
}

void test_resize_array_different_types() {
  printf("\n--- Testing samrena_resize_array with different types ---\n");
  Samrena *samrena = samrena_create_default();

  char *char_array = samrena_push(samrena, 10);
  assert(char_array != 0);
  strcpy(char_array, "hello");

  char *resized_char = samrena_resize_array(samrena, char_array, 10, 20);
  assert(resized_char != 0);
  assert(strcmp(resized_char, "hello") == 0);
  printf("Char array resize: '%s'\n", resized_char);

  double *double_array = samrena_push(samrena, 3 * sizeof(double));
  assert(double_array != 0);
  double_array[0] = 3.14;
  double_array[1] = 2.71;
  double_array[2] = 1.41;

  double *resized_double =
      samrena_resize_array(samrena, double_array, 3 * sizeof(double), 5 * sizeof(double));
  assert(resized_double != 0);
  assert(resized_double[0] == 3.14);
  assert(resized_double[1] == 2.71);
  assert(resized_double[2] == 1.41);
  printf("Double array resize preserved values\n");

  samrena_destroy(samrena);
}

void test_resize_array_capacity_exhaustion() {
  printf("\n--- Testing samrena_resize_array capacity exhaustion ---\n");
  Samrena *samrena = samrena_create_default();

  uint64_t large_size = samrena_capacity(samrena) - samrena_allocated(samrena) - 100;
  void *large_array = samrena_push(samrena, large_size);
  assert(large_array != 0);

  printf("Filled arena to near capacity\n");

  void *resized = samrena_resize_array(samrena, large_array, large_size, samrena_capacity(samrena));
  assert(resized != 0);
  printf("Resize beyond original capacity succeeded by growing arena\n");

  samrena_destroy(samrena);
}

void test_resize_array_alignment() {
  printf("\n--- Testing samrena_resize_array alignment preservation ---\n");
  Samrena *samrena = samrena_create_default();

  int64_t *int64_array = samrena_push(samrena, 3 * sizeof(int64_t));
  assert(int64_array != 0);
  assert(((uintptr_t)int64_array % sizeof(int64_t)) == 0);

  int64_array[0] = 0x1234567890ABCDEF;
  int64_array[1] = 0xFEDCBA0987654321;
  int64_array[2] = 0x0123456789ABCDEF;

  int64_t *resized_int64 =
      samrena_resize_array(samrena, int64_array, 3 * sizeof(int64_t), 6 * sizeof(int64_t));
  assert(resized_int64 != 0);
  assert(((uintptr_t)resized_int64 % sizeof(int64_t)) == 0);

  assert(resized_int64[0] == 0x1234567890ABCDEF);
  assert(resized_int64[1] == 0xFEDCBA0987654321);
  assert(resized_int64[2] == 0x0123456789ABCDEF);

  printf("int64_t array resize preserved alignment and data\n");

  samrena_destroy(samrena);
}
*/

int main(int argc, char **argv) {

  printf("START SAMRENA TESTING\n");

  int x = 42;
  int *ptr = &x;

  printf("Address of X %p\n", &x);
  printf("pointer? %p\n", (void *)ptr);

  create_new_arena();
  create_multiple_arrays();
  create_multiple_strings();

  // Run the new corner case tests
  test_zero_page_allocation();
  test_capacity_boundary();
  test_data_alignment();
  test_large_allocation();
  test_minimal_allocation();

  // Resize array tests disabled - samrena_resize_array function was removed
  // test_resize_array_basic();
  // test_resize_array_shrink();
  // test_resize_array_zero_sizes();
  // test_resize_array_different_types();
  // test_resize_array_capacity_exhaustion();
  // test_resize_array_alignment();

  printf("\nAll tests completed successfully!\n");
  return 0;
}
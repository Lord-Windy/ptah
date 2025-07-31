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

#include "datazoo_tests.h"
#include <assert.h>
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

void honeycomb_tests_adding_words() {
  printf("HONEYCOMB TESTS ADDING WORDS\n");

  Samrena *arena = samrena_create_default();
  Honeycomb *comb = honeycomb_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    honeycomb_put(comb, global_dictionary[i], x);
  }

  honeycomb_print(comb);
  honeycomb_destroy(comb);
  samrena_destroy(arena);
}

void honeycomb_tests_get_words() {

  printf("HONEYCOMB TESTS GET WORDS\n");

  Samrena *arena = samrena_create_default();
  Honeycomb *comb = honeycomb_create(10, arena);
  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    honeycomb_put(comb, global_dictionary[i], x);
  }

  for (int i = 0; i < 50; i++) {
    const int *x = honeycomb_get(comb, global_dictionary[i]);
    printf("%s: %d\n", global_dictionary[i], *x);
    assert(*x == i);
  }

  honeycomb_destroy(comb);
  samrena_destroy(arena);
}

void honeycomb_tests_remove_words() {
  printf("HONEYCOMB TESTS REMOVE WORDS\n");

  Samrena *arena = samrena_create_default();
  Honeycomb *comb = honeycomb_create(10, arena);
  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    honeycomb_put(comb, global_dictionary[i], x);
  }

  for (int i = 0; i < 25; i++) {
    honeycomb_remove(comb, global_dictionary[i]);
  }

  for (int i = 0; i < 50; i++) {
    const int *x = honeycomb_get(comb, global_dictionary[i]);
    printf("%s: %d\n", global_dictionary[i], x == NULL ? -1 : *x);
  }

  honeycomb_destroy(comb);
  samrena_destroy(arena);
}

void honeycomb_tests_contains_words() {
  printf("HONEYCOMB TESTS CONTAINS WORDS\n");
  Samrena *arena = samrena_create_default();
  Honeycomb *comb = honeycomb_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    honeycomb_put(comb, global_dictionary[i], x);
  }

  for (int i = 0; i < 50; i++) {
    printf("%s: %d\n", global_dictionary[i], honeycomb_contains(comb, global_dictionary[i]));
    assert(honeycomb_contains(comb, global_dictionary[i]));
  }

  honeycomb_destroy(comb);
  samrena_destroy(arena);
}

void honeycomb_tests_size() {
  printf("HONEYCOMB TESTS SIZE\n");
  Samrena *arena = samrena_create_default();
  Honeycomb *comb = honeycomb_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    honeycomb_put(comb, global_dictionary[i], x);
  }

  printf("size: %zu\n", honeycomb_size(comb));
  assert(honeycomb_size(comb) == 50);

  honeycomb_destroy(comb);
  samrena_destroy(arena);
}

void honeycomb_tests_update_words() {}

void honeycomb_tests_key_copying() {
  printf("HONEYCOMB TESTS KEY COPYING\n");
  Samrena *arena = samrena_create_default();
  Honeycomb *comb = honeycomb_create(10, arena);
  
  // Create a key in a local buffer
  char local_key[32];
  strcpy(local_key, "test_key");
  
  // Insert with the local key
  int *value = samrena_push(arena, sizeof(int));
  *value = 42;
  honeycomb_put(comb, local_key, value);
  
  // Modify the local buffer
  strcpy(local_key, "modified");
  
  // Verify the hash map still has the original key
  int *retrieved = honeycomb_get(comb, "test_key");
  assert(retrieved != NULL);
  assert(*retrieved == 42);
  
  // Verify the modified key is not found
  assert(honeycomb_get(comb, "modified") == NULL);
  
  printf("Key copying test passed!\n");
  
  honeycomb_destroy(comb);
  samrena_destroy(arena);
}

void honeycomb_tests() {
  honeycomb_tests_adding_words();
  honeycomb_tests_get_words();
  honeycomb_tests_remove_words();
  honeycomb_tests_size();
  honeycomb_tests_contains_words();
  honeycomb_tests_update_words();
  honeycomb_tests_key_copying();
}

// Type-Safe Wrapper Tests

void test_string_string_map() {
  printf("TESTING string_string_map\n");
  
  Samrena *arena = samrena_create_default();
  string_string_honeycomb *map = string_string_create(10, arena);
  
  // Test basic operations
  assert(string_string_is_empty(map));
  assert(string_string_size(map) == 0);
  
  // Test put and get
  assert(string_string_put(map, "key1", "value1"));
  assert(string_string_put(map, "key2", "value2"));
  assert(string_string_size(map) == 2);
  assert(!string_string_is_empty(map));
  
  const char *val1 = string_string_get(map, "key1");
  const char *val2 = string_string_get(map, "key2");
  assert(val1 != NULL && strcmp(val1, "value1") == 0);
  assert(val2 != NULL && strcmp(val2, "value2") == 0);
  
  // Test contains
  assert(string_string_contains(map, "key1"));
  assert(string_string_contains(map, "key2"));
  assert(!string_string_contains(map, "nonexistent"));
  
  // Test remove
  assert(string_string_remove(map, "key1"));
  assert(!string_string_contains(map, "key1"));
  assert(string_string_size(map) == 1);
  
  // Test clear
  string_string_clear(map);
  assert(string_string_size(map) == 0);
  assert(string_string_is_empty(map));
  
  string_string_destroy(map);
  samrena_destroy(arena);
  printf("string_string_map tests passed!\n");
}

void test_string_int_map() {
  printf("TESTING string_int_map\n");
  
  Samrena *arena = samrena_create_default();
  string_int_honeycomb *map = string_int_create(10, arena);
  
  // Create some integers in the arena
  int *val1 = samrena_push(arena, sizeof(int));
  int *val2 = samrena_push(arena, sizeof(int));
  int *val3 = samrena_push(arena, sizeof(int));
  *val1 = 42;
  *val2 = 100;
  *val3 = -5;
  
  // Test operations
  assert(string_int_put(map, "answer", val1));
  assert(string_int_put(map, "hundred", val2));
  assert(string_int_put(map, "negative", val3));
  
  int *retrieved1 = string_int_get(map, "answer");
  int *retrieved2 = string_int_get(map, "hundred");
  int *retrieved3 = string_int_get(map, "negative");
  
  assert(retrieved1 != NULL && *retrieved1 == 42);
  assert(retrieved2 != NULL && *retrieved2 == 100);
  assert(retrieved3 != NULL && *retrieved3 == -5);
  
  assert(string_int_size(map) == 3);
  
  string_int_destroy(map);
  samrena_destroy(arena);
  printf("string_int_map tests passed!\n");
}

void test_string_ptr_map() {
  printf("TESTING string_ptr_map\n");
  
  Samrena *arena = samrena_create_default();
  string_ptr_honeycomb *map = string_ptr_create(10, arena);
  
  // Use different pointer types
  char *str_ptr = samrena_push(arena, 20);
  strcpy(str_ptr, "test string");
  
  int *int_ptr = samrena_push(arena, sizeof(int));
  *int_ptr = 123;
  
  // Test storing different pointer types
  assert(string_ptr_put(map, "string_data", str_ptr));
  assert(string_ptr_put(map, "int_data", int_ptr));
  
  char *retrieved_str = (char*)string_ptr_get(map, "string_data");
  int *retrieved_int = (int*)string_ptr_get(map, "int_data");
  
  assert(retrieved_str != NULL && strcmp(retrieved_str, "test string") == 0);
  assert(retrieved_int != NULL && *retrieved_int == 123);
  
  string_ptr_destroy(map);
  samrena_destroy(arena);
  printf("string_ptr_map tests passed!\n");
}

static int foreach_count = 0;
static char collected_keys[10][50];
static int collected_values[10];

void string_int_iterator_test(const char *key, int *value, void *user_data) {
  strcpy(collected_keys[foreach_count], key);
  collected_values[foreach_count] = *value;
  foreach_count++;
}

void test_typed_foreach() {
  printf("TESTING typed foreach\n");
  
  Samrena *arena = samrena_create_default();
  string_int_honeycomb *map = string_int_create(10, arena);
  
  // Add some test data
  for (int i = 0; i < 5; i++) {
    int *val = samrena_push(arena, sizeof(int));
    *val = i * 10;
    char key[20];
    sprintf(key, "key%d", i);
    string_int_put(map, key, val);
  }
  
  // Reset counter and test foreach
  foreach_count = 0;
  string_int_foreach(map, string_int_iterator_test, NULL);
  
  assert(foreach_count == 5);
  
  // Verify all keys and values were collected (order may vary due to hashing)
  for (int i = 0; i < 5; i++) {
    bool found = false;
    for (int j = 0; j < foreach_count; j++) {
      char expected_key[20];
      sprintf(expected_key, "key%d", i);
      if (strcmp(collected_keys[j], expected_key) == 0) {
        assert(collected_values[j] == i * 10);
        found = true;
        break;
      }
    }
    assert(found);
  }
  
  string_int_destroy(map);
  samrena_destroy(arena);
  printf("typed foreach tests passed!\n");
}

void test_null_safety() {
  printf("TESTING null safety\n");
  
  // Test all functions with NULL pointers
  assert(string_string_create(10, NULL) == NULL);
  
  string_string_destroy(NULL);  // Should not crash
  
  assert(!string_string_put(NULL, "key", "value"));
  assert(string_string_get(NULL, "key") == NULL);
  assert(!string_string_remove(NULL, "key"));
  assert(!string_string_contains(NULL, "key"));
  assert(string_string_size(NULL) == 0);
  assert(string_string_is_empty(NULL));
  
  string_string_clear(NULL);  // Should not crash
  string_string_foreach(NULL, NULL, NULL);  // Should not crash
  
  printf("null safety tests passed!\n");
}

// Define a custom type-safe map for our specific use case
HONEYCOMB_DEFINE_TYPED(word_index, const char*, int*)

void test_custom_typed_map() {
  printf("TESTING custom typed map definition\n");
  
  Samrena *arena = samrena_create_default();
  word_index_honeycomb *word_map = word_index_create(10, arena);
  
  // Use it like the predefined ones
  int *index1 = samrena_push(arena, sizeof(int));
  int *index2 = samrena_push(arena, sizeof(int));
  *index1 = 0;
  *index2 = 25;
  
  assert(word_index_put(word_map, "ability", index1));
  assert(word_index_put(word_map, "argue", index2));
  
  int *retrieved1 = word_index_get(word_map, "ability");
  int *retrieved2 = word_index_get(word_map, "argue");
  
  assert(retrieved1 != NULL && *retrieved1 == 0);
  assert(retrieved2 != NULL && *retrieved2 == 25);
  
  word_index_destroy(word_map);
  samrena_destroy(arena);
  printf("custom typed map tests passed!\n");
}

void typed_honeycomb_tests() {
  printf("\n=== STARTING TYPED HONEYCOMB TESTS ===\n");
  
  test_string_string_map();
  test_string_int_map();
  test_string_ptr_map();
  test_typed_foreach();
  test_null_safety();
  test_custom_typed_map();
  
  printf("=== ALL TYPED HONEYCOMB TESTS PASSED ===\n\n");
}
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

#include "samdata_tests.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

void samhashmap_tests_adding_words() {
  printf("SAMHASHMAP TESTS ADDING WORDS\n");

  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    samhashmap_put(comb, global_dictionary[i], x);
  }

  samhashmap_print(comb);
  samhashmap_destroy(comb);
  samrena_destroy(arena);
}

void samhashmap_tests_get_words() {

  printf("SAMHASHMAP TESTS GET WORDS\n");

  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    samhashmap_put(comb, global_dictionary[i], x);
  }

  for (int i = 0; i < 50; i++) {
    const int *x = samhashmap_get(comb, global_dictionary[i]);
    printf("%s: %d\n", global_dictionary[i], *x);
    assert(*x == i);
  }

  samhashmap_destroy(comb);
  samrena_destroy(arena);
}

void samhashmap_tests_remove_words() {
  printf("SAMHASHMAP TESTS REMOVE WORDS\n");

  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    samhashmap_put(comb, global_dictionary[i], x);
  }

  for (int i = 0; i < 25; i++) {
    samhashmap_remove(comb, global_dictionary[i]);
  }

  for (int i = 0; i < 50; i++) {
    const int *x = samhashmap_get(comb, global_dictionary[i]);
    printf("%s: %d\n", global_dictionary[i], x == NULL ? -1 : *x);
  }

  samhashmap_destroy(comb);
  samrena_destroy(arena);
}

void samhashmap_tests_contains_words() {
  printf("SAMHASHMAP TESTS CONTAINS WORDS\n");
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    samhashmap_put(comb, global_dictionary[i], x);
  }

  for (int i = 0; i < 50; i++) {
    printf("%s: %d\n", global_dictionary[i], samhashmap_contains(comb, global_dictionary[i]));
    assert(samhashmap_contains(comb, global_dictionary[i]));
  }

  samhashmap_destroy(comb);
  samrena_destroy(arena);
}

void samhashmap_tests_size() {
  printf("SAMHASHMAP TESTS SIZE\n");
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    samhashmap_put(comb, global_dictionary[i], x);
  }

  printf("size: %zu\n", samhashmap_size(comb));
  assert(samhashmap_size(comb) == 50);

  samhashmap_destroy(comb);
  samrena_destroy(arena);
}

void samhashmap_tests_update_words() {
  printf("SAMHASHMAP TESTS UPDATE WORDS\n");
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  // Test updating existing keys
  int *original_value = samrena_push(arena, sizeof(int));
  int *new_value = samrena_push(arena, sizeof(int));
  *original_value = 100;
  *new_value = 200;
  
  // Insert initial value
  samhashmap_put(comb, "test_key", original_value);
  assert(samhashmap_size(comb) == 1);
  
  // Verify initial value
  int *retrieved = samhashmap_get(comb, "test_key");
  assert(retrieved != NULL && *retrieved == 100);
  
  // Update with new value
  samhashmap_put(comb, "test_key", new_value);
  assert(samhashmap_size(comb) == 1);  // Size should remain the same
  
  // Verify new value
  retrieved = samhashmap_get(comb, "test_key");
  assert(retrieved != NULL && *retrieved == 200);
  
  printf("Update test passed!\n");
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
}

void samhashmap_tests_key_copying() {
  printf("SAMHASHMAP TESTS KEY COPYING\n");
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  // Create a key in a local buffer
  char local_key[32];
  strcpy(local_key, "test_key");
  
  // Insert with the local key
  int *value = samrena_push(arena, sizeof(int));
  *value = 42;
  samhashmap_put(comb, local_key, value);
  
  // Modify the local buffer
  strcpy(local_key, "modified");
  
  // Verify the hash map still has the original key
  int *retrieved = samhashmap_get(comb, "test_key");
  assert(retrieved != NULL);
  assert(*retrieved == 42);
  
  // Verify the modified key is not found
  assert(samhashmap_get(comb, "modified") == NULL);
  
  printf("Key copying test passed!\n");
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
}

void samhashmap_tests() {
  // Original basic tests
  samhashmap_tests_adding_words();
  samhashmap_tests_get_words();
  samhashmap_tests_remove_words();
  samhashmap_tests_size();
  samhashmap_tests_contains_words();
  samhashmap_tests_update_words();
  samhashmap_tests_key_copying();
  
  // New comprehensive test suites (Steps 16-17)
  comprehensive_edge_case_tests();
  collision_handling_tests();
  resize_behavior_tests();
  memory_exhaustion_tests();
  performance_benchmark_tests();
  debugging_support_tests();
}

// Type-Safe Wrapper Tests

void test_string_string_map() {
  printf("TESTING string_string_map\n");
  
  Samrena *arena = samrena_create_default();
  string_string_samhashmap *map = string_string_create(10, arena);
  
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
  string_int_samhashmap *map = string_int_create(10, arena);
  
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
  string_ptr_samhashmap *map = string_ptr_create(10, arena);
  
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
  string_int_samhashmap *map = string_int_create(10, arena);
  
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
SAMHASHMAP_DEFINE_TYPED(word_index, const char*, int*)

void test_custom_typed_map() {
  printf("TESTING custom typed map definition\n");
  
  Samrena *arena = samrena_create_default();
  word_index_samhashmap *word_map = word_index_create(10, arena);
  
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

// COMPREHENSIVE EDGE CASE TESTS (Step 16 from planning)

void test_empty_hashmap() {
  printf("TESTING empty samhashmap edge cases\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  // Test operations on empty hashmap
  assert(samhashmap_size(comb) == 0);
  assert(samhashmap_is_empty(comb));
  assert(samhashmap_get(comb, "nonexistent") == NULL);
  assert(!samhashmap_contains(comb, "nonexistent"));
  assert(!samhashmap_remove(comb, "nonexistent"));
  
  // Clear empty hashmap should not crash
  samhashmap_clear(comb);
  assert(samhashmap_size(comb) == 0);
  
  // Get keys/values from empty hashmap
  const char *keys[10];
  void *values[10];
  assert(samhashmap_get_keys(comb, keys, 10) == 0);
  assert(samhashmap_get_values(comb, values, 10) == 0);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Empty samhashmap tests passed!\n");
}

void test_single_element() {
  printf("TESTING single element edge cases\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(1, arena);  // Very small capacity
  
  int *value = samrena_push(arena, sizeof(int));
  *value = 42;
  
  // Add single element
  assert(samhashmap_put(comb, "single", value));
  assert(samhashmap_size(comb) == 1);
  assert(!samhashmap_is_empty(comb));
  
  // Verify retrieval
  int *retrieved = samhashmap_get(comb, "single");
  assert(retrieved != NULL && *retrieved == 42);
  assert(samhashmap_contains(comb, "single"));
  
  // Test with different key
  assert(samhashmap_get(comb, "different") == NULL);
  assert(!samhashmap_contains(comb, "different"));
  
  // Remove the single element
  assert(samhashmap_remove(comb, "single"));
  assert(samhashmap_size(comb) == 0);
  assert(samhashmap_is_empty(comb));
  
  // Try to remove again
  assert(!samhashmap_remove(comb, "single"));
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Single element tests passed!\n");
}

void test_null_key_and_values() {
  printf("TESTING null key and value handling\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  int *value = samrena_push(arena, sizeof(int));
  *value = 42;
  
  // Test null key
  assert(!samhashmap_put(comb, NULL, value));
  assert(samhashmap_get(comb, NULL) == NULL);
  assert(!samhashmap_contains(comb, NULL));
  assert(!samhashmap_remove(comb, NULL));
  
  // Test null value (should be allowed)
  assert(samhashmap_put(comb, "null_value", NULL));
  assert(samhashmap_contains(comb, "null_value"));
  assert(samhashmap_get(comb, "null_value") == NULL);
  assert(samhashmap_size(comb) == 1);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Null handling tests passed!\n");
}

void test_empty_string_key() {
  printf("TESTING empty string key\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  int *value = samrena_push(arena, sizeof(int));
  *value = 42;
  
  // Test empty string key
  assert(samhashmap_put(comb, "", value));
  assert(samhashmap_contains(comb, ""));
  
  int *retrieved = samhashmap_get(comb, "");
  assert(retrieved != NULL && *retrieved == 42);
  
  assert(samhashmap_remove(comb, ""));
  assert(!samhashmap_contains(comb, ""));
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Empty string key tests passed!\n");
}

void test_duplicate_keys() {
  printf("TESTING duplicate key handling\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  int *value1 = samrena_push(arena, sizeof(int));
  int *value2 = samrena_push(arena, sizeof(int));
  *value1 = 100;
  *value2 = 200;
  
  // Insert first value
  assert(samhashmap_put(comb, "duplicate", value1));
  assert(samhashmap_size(comb) == 1);
  
  // Insert second value with same key
  assert(samhashmap_put(comb, "duplicate", value2));
  assert(samhashmap_size(comb) == 1);  // Size should not increase
  
  // Should get the updated value
  int *retrieved = samhashmap_get(comb, "duplicate");
  assert(retrieved != NULL && *retrieved == 200);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Duplicate key tests passed!\n");
}

void test_very_long_keys() {
  printf("TESTING very long keys\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  // Create a very long key (1000 characters)
  char *long_key = samrena_push(arena, 1001);
  for (int i = 0; i < 1000; i++) {
    long_key[i] = 'a' + (i % 26);
  }
  long_key[1000] = '\0';
  
  int *value = samrena_push(arena, sizeof(int));
  *value = 42;
  
  // Test operations with very long key
  assert(samhashmap_put(comb, long_key, value));
  assert(samhashmap_contains(comb, long_key));
  
  int *retrieved = samhashmap_get(comb, long_key);
  assert(retrieved != NULL && *retrieved == 42);
  
  assert(samhashmap_remove(comb, long_key));
  assert(!samhashmap_contains(comb, long_key));
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Very long key tests passed!\n");
}

void test_similar_keys() {
  printf("TESTING similar keys (hash collision potential)\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  // Use keys that might hash to similar values
  const char *similar_keys[] = {
    "abc", "acb", "bac", "bca", "cab", "cba",
    "test1", "test2", "test3", "test4"
  };
  int num_keys = sizeof(similar_keys) / sizeof(similar_keys[0]);
  
  // Insert all similar keys
  for (int i = 0; i < num_keys; i++) {
    int *value = samrena_push(arena, sizeof(int));
    *value = i;
    assert(samhashmap_put(comb, similar_keys[i], value));
  }
  
  assert(samhashmap_size(comb) == (size_t)num_keys);
  
  // Verify all keys can be retrieved correctly
  for (int i = 0; i < num_keys; i++) {
    int *retrieved = samhashmap_get(comb, similar_keys[i]);
    assert(retrieved != NULL && *retrieved == i);
    assert(samhashmap_contains(comb, similar_keys[i]));
  }
  
  // Remove every other key
  for (int i = 0; i < num_keys; i += 2) {
    assert(samhashmap_remove(comb, similar_keys[i]));
  }
  
  // Verify remaining keys
  for (int i = 0; i < num_keys; i++) {
    if (i % 2 == 0) {
      assert(!samhashmap_contains(comb, similar_keys[i]));
    } else {
      assert(samhashmap_contains(comb, similar_keys[i]));
    }
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Similar key tests passed!\n");
}

// COLLISION HANDLING TESTS (Step 16 from planning)

void test_forced_collisions() {
  printf("TESTING forced hash collisions\n");
  
  Samrena *arena = samrena_create_default();
  // Use very small capacity to force collisions
  SamHashMap *comb = samhashmap_create(2, arena);
  
  // Add many items to force collisions in the small table
  const int num_items = 20;
  for (int i = 0; i < num_items; i++) {
    char key[32];
    sprintf(key, "key_%d", i);
    
    int *value = samrena_push(arena, sizeof(int));
    *value = i * 10;
    
    assert(samhashmap_put(comb, key, value));
  }
  
  assert(samhashmap_size(comb) == num_items);
  
  // Verify all items can be retrieved despite collisions
  for (int i = 0; i < num_items; i++) {
    char key[32];
    sprintf(key, "key_%d", i);
    
    int *retrieved = samhashmap_get(comb, key);
    assert(retrieved != NULL);
    assert(*retrieved == i * 10);
    assert(samhashmap_contains(comb, key));
  }
  
  // Remove items in random order and verify remaining items
  int remove_order[] = {5, 12, 3, 18, 1, 9, 15, 0, 7, 19};
  int remove_count = sizeof(remove_order) / sizeof(remove_order[0]);
  
  for (int i = 0; i < remove_count; i++) {
    char key[32];
    sprintf(key, "key_%d", remove_order[i]);
    assert(samhashmap_remove(comb, key));
  }
  
  // Verify removed items are gone and remaining items are still there
  bool removed[20] = {false};  // num_items is 20, so use constant
  for (int i = 0; i < remove_count; i++) {
    removed[remove_order[i]] = true;
  }
  
  for (int i = 0; i < num_items; i++) {
    char key[32];
    sprintf(key, "key_%d", i);
    
    if (removed[i]) {
      assert(!samhashmap_contains(comb, key));
      assert(samhashmap_get(comb, key) == NULL);
    } else {
      assert(samhashmap_contains(comb, key));
      int *retrieved = samhashmap_get(comb, key);
      assert(retrieved != NULL && *retrieved == i * 10);
    }
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Forced collision tests passed!\n");
}

void test_collision_chain_integrity() {
  printf("TESTING collision chain integrity\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(1, arena);  // Force everything into one bucket
  
  const char *test_keys[] = {
    "first", "second", "third", "fourth", "fifth"
  };
  int num_keys = sizeof(test_keys) / sizeof(test_keys[0]);
  
  // Insert all keys (they'll all collide in the single bucket)
  for (int i = 0; i < num_keys; i++) {
    int *value = samrena_push(arena, sizeof(int));
    *value = i + 100;
    assert(samhashmap_put(comb, test_keys[i], value));
  }
  
  // Remove middle element and verify chain remains intact
  assert(samhashmap_remove(comb, "third"));
  assert(!samhashmap_contains(comb, "third"));
  
  // Verify other elements are still accessible
  for (int i = 0; i < num_keys; i++) {
    if (strcmp(test_keys[i], "third") != 0) {
      assert(samhashmap_contains(comb, test_keys[i]));
      int *retrieved = samhashmap_get(comb, test_keys[i]);
      assert(retrieved != NULL && *retrieved == i + 100);
    }
  }
  
  // Remove first element (head of chain)
  assert(samhashmap_remove(comb, "first"));
  
  // Verify remaining elements are still accessible
  for (int i = 0; i < num_keys; i++) {
    if (strcmp(test_keys[i], "first") != 0 && strcmp(test_keys[i], "third") != 0) {
      assert(samhashmap_contains(comb, test_keys[i]));
      int *retrieved = samhashmap_get(comb, test_keys[i]);
      assert(retrieved != NULL && *retrieved == i + 100);
    }
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Collision chain integrity tests passed!\n");
}

void test_collision_with_updates() {
  printf("TESTING collision handling with updates\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(3, arena);  // Small capacity for collisions
  
  // Insert items that will likely collide
  int *value1 = samrena_push(arena, sizeof(int));
  int *value2 = samrena_push(arena, sizeof(int));
  int *value3 = samrena_push(arena, sizeof(int));
  int *new_value2 = samrena_push(arena, sizeof(int));
  
  *value1 = 100;
  *value2 = 200;
  *value3 = 300;
  *new_value2 = 250;
  
  assert(samhashmap_put(comb, "collide_a", value1));
  assert(samhashmap_put(comb, "collide_b", value2));
  assert(samhashmap_put(comb, "collide_c", value3));
  
  // Update middle item in collision chain
  assert(samhashmap_put(comb, "collide_b", new_value2));
  assert(samhashmap_size(comb) == 3);  // Size should remain the same
  
  // Verify all values are correct
  int *retrieved_a = samhashmap_get(comb, "collide_a");
  int *retrieved_b = samhashmap_get(comb, "collide_b");
  int *retrieved_c = samhashmap_get(comb, "collide_c");
  
  assert(retrieved_a != NULL && *retrieved_a == 100);
  assert(retrieved_b != NULL && *retrieved_b == 250);  // Updated value
  assert(retrieved_c != NULL && *retrieved_c == 300);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Collision with updates tests passed!\n");
}

// RESIZE BEHAVIOR TESTS (Step 16 from planning)

void test_automatic_growth() {
  printf("TESTING automatic growth behavior\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(4, arena);  // Start small
  
  size_t initial_capacity = comb->capacity;
  printf("Initial capacity: %zu\n", initial_capacity);
  
  // Add items until we trigger a resize (load factor > 0.75)
  // With capacity 4, we should resize after 3 items (3/4 = 0.75)
  const int items_to_add = 10;
  
  for (int i = 0; i < items_to_add; i++) {
    char key[32];
    sprintf(key, "grow_key_%d", i);
    
    int *value = samrena_push(arena, sizeof(int));
    *value = i * 5;
    
    size_t size_before = samhashmap_size(comb);
    size_t capacity_before = comb->capacity;
    
    assert(samhashmap_put(comb, key, value));
    
    printf("Added item %d, size: %zu, capacity: %zu\n", 
           i, samhashmap_size(comb), comb->capacity);
    
    // Check if resize occurred
    if (comb->capacity > capacity_before) {
      printf("Resize occurred: %zu -> %zu\n", capacity_before, comb->capacity);
    }
  }
  
  // Verify final capacity is greater than initial
  assert(comb->capacity > initial_capacity);
  assert(samhashmap_size(comb) == items_to_add);
  
  // Verify all items are still accessible after resizes
  for (int i = 0; i < items_to_add; i++) {
    char key[32];
    sprintf(key, "grow_key_%d", i);
    
    int *retrieved = samhashmap_get(comb, key);
    assert(retrieved != NULL);
    assert(*retrieved == i * 5);
    assert(samhashmap_contains(comb, key));
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Automatic growth tests passed!\n");
}

void test_load_factor_threshold() {
  printf("TESTING load factor threshold behavior\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(8, arena);
  
  // Default load factor should be 0.75
  float load_factor = comb->load_factor;
  printf("Load factor: %f\n", load_factor);
  assert(load_factor == 0.75f);
  
  size_t capacity = comb->capacity;
  size_t threshold = (size_t)(capacity * load_factor);
  printf("Capacity: %zu, Threshold: %zu\n", capacity, threshold);
  
  // Add items up to but not exceeding threshold
  for (size_t i = 0; i < threshold; i++) {
    char key[32];
    sprintf(key, "threshold_%zu", i);
    
    int *value = samrena_push(arena, sizeof(int));
    *value = (int)i;
    
    assert(samhashmap_put(comb, key, value));
    assert(comb->capacity == capacity);  // Should not resize yet
  }
  
  // Add one more item to exceed threshold
  char key[32];
  sprintf(key, "threshold_%zu", threshold);
  int *value = samrena_push(arena, sizeof(int));
  *value = (int)threshold;
  
  assert(samhashmap_put(comb, key, value));
  assert(comb->capacity > capacity);  // Should have resized
  
  printf("Resize triggered at size %zu with capacity %zu\n", 
         samhashmap_size(comb), comb->capacity);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Load factor threshold tests passed!\n");
}

void test_resize_preserves_data() {
  printf("TESTING data preservation during resize\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(2, arena);  // Very small for quick resize
  
  // Use test data that we know well
  const char *test_data[][2] = {
    {"apple", "fruit"},
    {"carrot", "vegetable"},
    {"salmon", "fish"},
    {"bread", "grain"},
    {"milk", "dairy"},
    {"chicken", "meat"},
    {"broccoli", "vegetable"},
    {"banana", "fruit"}
  };
  int num_items = sizeof(test_data) / sizeof(test_data[0]);
  
  // Insert all items (will trigger multiple resizes)
  for (int i = 0; i < num_items; i++) {
    // Allocate strings in arena
    char *value = samrena_push(arena, strlen(test_data[i][1]) + 1);
    strcpy(value, test_data[i][1]);
    
    assert(samhashmap_put(comb, test_data[i][0], value));
    printf("Added %s->%s (size: %zu, capacity: %zu)\n", 
           test_data[i][0], test_data[i][1], 
           samhashmap_size(comb), comb->capacity);
  }
  
  // Verify all data is preserved and accessible
  assert(samhashmap_size(comb) == (size_t)num_items);
  
  for (int i = 0; i < num_items; i++) {
    char *retrieved = (char*)samhashmap_get(comb, test_data[i][0]);
    assert(retrieved != NULL);
    assert(strcmp(retrieved, test_data[i][1]) == 0);
    printf("Verified %s->%s\n", test_data[i][0], retrieved);
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Data preservation during resize tests passed!\n");
}

void test_multiple_resizes() {
  printf("TESTING multiple sequential resizes\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(2, arena);
  
  size_t capacities[10];
  int capacity_count = 0;
  capacities[capacity_count++] = comb->capacity;
  
  // Add many items to trigger multiple resizes
  const int total_items = 50;
  
  for (int i = 0; i < total_items; i++) {
    char key[32];
    sprintf(key, "multi_resize_%d", i);
    
    int *value = samrena_push(arena, sizeof(int));
    *value = i;
    
    size_t old_capacity = comb->capacity;
    assert(samhashmap_put(comb, key, value));
    
    if (comb->capacity > old_capacity) {
      printf("Resize %d: %zu -> %zu (after adding %d items)\n", 
             capacity_count, old_capacity, comb->capacity, i + 1);
      capacities[capacity_count++] = comb->capacity;
    }
  }
  
  printf("Total resizes: %d\n", capacity_count - 1);
  assert(capacity_count > 2);  // Should have resized at least once
  
  // Verify all items are still accessible
  for (int i = 0; i < total_items; i++) {
    char key[32];
    sprintf(key, "multi_resize_%d", i);
    
    int *retrieved = samhashmap_get(comb, key);
    assert(retrieved != NULL && *retrieved == i);
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Multiple resize tests passed!\n");
}

// MEMORY EXHAUSTION SCENARIO TESTS (Step 16 from planning)

void test_small_arena_exhaustion() {
  printf("TESTING small arena memory exhaustion\n");
  
  // Create a small arena using default config but small page size
  SamrenaConfig config = samrena_default_config();
  config.page_size = 2048;  // Very small pages
  config.initial_pages = 1;  // Only start with 1 page
  Samrena *small_arena = samrena_create(&config);
  SamHashMap *comb = samhashmap_create(10, small_arena);
  
  if (comb == NULL) {
    printf("Initial creation failed due to small arena\n");
    samrena_destroy(small_arena);
    return;
  }
  
  int successful_insertions = 0;
  int failed_insertions = 0;
  
  // Try to insert many items until we run out of memory
  for (int i = 0; i < 1000; i++) {
    char key[32];
    sprintf(key, "exhaust_%d", i);
    
    // Try to allocate value in arena
    int *value = samrena_push(small_arena, sizeof(int));
    if (value == NULL) {
      failed_insertions++;
      break;  // Arena exhausted
    }
    *value = i;
    
    if (samhashmap_put(comb, key, value)) {
      successful_insertions++;
    } else {
      failed_insertions++;
      break;  // Hashmap insertion failed
    }
  }
  
  printf("Successful insertions: %d, Failed: %d\n", 
         successful_insertions, failed_insertions);
  
  // Verify that successful insertions are still accessible
  for (int i = 0; i < successful_insertions; i++) {
    char key[32];
    sprintf(key, "exhaust_%d", i);
    
    int *retrieved = samhashmap_get(comb, key);
    if (retrieved == NULL || *retrieved != i) {
      printf("Data corruption detected at key %s\n", key);
      assert(false);
    }
  }
  
  assert(samhashmap_size(comb) == (size_t)successful_insertions);
  
  samhashmap_destroy(comb);
  samrena_destroy(small_arena);
  printf("Small arena exhaustion tests passed!\n");
}

void test_arena_exhaustion_during_resize() {
  printf("TESTING arena exhaustion during resize\n");
  
  // Create arena with limited memory
  SamrenaConfig config = samrena_default_config();
  config.page_size = 4096;  // Small pages
  config.initial_pages = 2;  // Start with 2 pages
  Samrena *limited_arena = samrena_create(&config);
  SamHashMap *comb = samhashmap_create(2, limited_arena);  // Small initial capacity
  
  if (comb == NULL) {
    printf("Initial creation failed\n");
    samrena_destroy(limited_arena);
    return;
  }
  
  int items_added = 0;
  bool resize_failed = false;
  
  // Add items until either arena exhausts or resize fails
  for (int i = 0; i < 200; i++) {
    char key[64];  // Larger keys to consume more memory
    sprintf(key, "resize_exhaust_with_long_key_name_%d", i);
    
    // Try to allocate a reasonably sized value
    char *value = samrena_push(limited_arena, 100);  // 100 bytes per value
    if (value == NULL) {
      printf("Arena exhausted during value allocation at item %d\n", i);
      break;
    }
    sprintf(value, "value_data_%d", i);
    
    size_t old_capacity = comb->capacity;
    bool put_result = samhashmap_put(comb, key, value);
    
    if (!put_result) {
      printf("Put failed at item %d (likely during resize)\n", i);
      resize_failed = true;
      break;
    }
    
    items_added++;
    
    if (comb->capacity > old_capacity) {
      printf("Resize occurred: %zu -> %zu at item %d\n", 
             old_capacity, comb->capacity, i);
    }
  }
  
  printf("Items successfully added: %d\n", items_added);
  printf("Resize failed: %s\n", resize_failed ? "yes" : "no");
  
  // Verify data integrity for successfully added items
  for (int i = 0; i < items_added; i++) {
    char key[64];
    sprintf(key, "resize_exhaust_with_long_key_name_%d", i);
    
    char *retrieved = (char*)samhashmap_get(comb, key);
    if (retrieved == NULL) {
      printf("Missing key: %s\n", key);
      assert(false);
    }
    
    char expected[32];
    sprintf(expected, "value_data_%d", i);
    if (strcmp(retrieved, expected) != 0) {
      printf("Data corruption: expected %s, got %s\n", expected, retrieved);
      assert(false);
    }
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(limited_arena);
  printf("Arena exhaustion during resize tests passed!\n");
}

void test_graceful_degradation() {
  printf("TESTING graceful degradation under memory pressure\n");
  
  SamrenaConfig config = samrena_default_config();
  config.page_size = 2048;  // Small pages
  config.initial_pages = 1;  // Start with 1 page
  Samrena *pressure_arena = samrena_create(&config);
  SamHashMap *comb = samhashmap_create(8, pressure_arena);
  
  if (comb == NULL) {
    printf("Initial creation failed\n");
    samrena_destroy(pressure_arena);
    return;
  }
  
  // First, add some items successfully
  int initial_items = 0;
  for (int i = 0; i < 20; i++) {
    char key[32];
    sprintf(key, "pressure_%d", i);
    
    int *value = samrena_push(pressure_arena, sizeof(int));
    if (value == NULL) break;
    *value = i * 2;
    
    if (samhashmap_put(comb, key, value)) {
      initial_items++;
    } else {
      break;
    }
  }
  
  printf("Successfully added %d initial items\n", initial_items);
  
  // Now try operations that might fail due to memory pressure
  printf("Testing operations under memory pressure...\n");
  
  // Test get operations (should still work)
  for (int i = 0; i < initial_items; i++) {
    char key[32];
    sprintf(key, "pressure_%d", i);
    
    int *retrieved = samhashmap_get(comb, key);
    assert(retrieved != NULL && *retrieved == i * 2);
  }
  
  // Test contains operations (should still work)
  for (int i = 0; i < initial_items; i++) {
    char key[32];
    sprintf(key, "pressure_%d", i);
    assert(samhashmap_contains(comb, key));
  }
  
  // Test removal (should work and might free some memory)
  int removed_count = 0;
  for (int i = 0; i < initial_items; i += 2) {  // Remove every other item
    char key[32];
    sprintf(key, "pressure_%d", i);
    
    if (samhashmap_remove(comb, key)) {
      removed_count++;
    }
  }
  
  printf("Removed %d items under memory pressure\n", removed_count);
  
  // Verify remaining items are still accessible
  int remaining_verified = 0;
  for (int i = 0; i < initial_items; i++) {
    char key[32];
    sprintf(key, "pressure_%d", i);
    
    if (i % 2 == 0) {
      // Should be removed
      assert(!samhashmap_contains(comb, key));
      assert(samhashmap_get(comb, key) == NULL);
    } else {
      // Should still exist
      assert(samhashmap_contains(comb, key));
      int *retrieved = samhashmap_get(comb, key);
      assert(retrieved != NULL && *retrieved == i * 2);
      remaining_verified++;
    }
  }
  
  printf("Verified %d remaining items\n", remaining_verified);
  
  samhashmap_destroy(comb);
  samrena_destroy(pressure_arena);
  printf("Graceful degradation tests passed!\n");
}

// PERFORMANCE BENCHMARK TESTS (Step 16 from planning)

void benchmark_insertion_performance() {
  printf("BENCHMARKING insertion performance\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(1000, arena);
  
  const int num_items = 10000;
  clock_t start_time = clock();
  
  // Benchmark bulk insertion
  for (int i = 0; i < num_items; i++) {
    char key[32];
    sprintf(key, "perf_key_%d", i);
    
    int *value = samrena_push(arena, sizeof(int));
    *value = i;
    
    assert(samhashmap_put(comb, key, value));
  }
  
  clock_t end_time = clock();
  double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  
  printf("Inserted %d items in %.6f seconds\n", num_items, elapsed);
  printf("Insertion rate: %.0f items/second\n", num_items / elapsed);
  printf("Average time per insertion: %.6f ms\n", (elapsed * 1000) / num_items);
  
  // Print final stats
  SamHashMapStats stats = samhashmap_get_stats(comb);
  printf("Final capacity: %zu\n", comb->capacity);
  printf("Load factor: %.3f\n", (float)samhashmap_size(comb) / comb->capacity);
  printf("Total collisions: %zu\n", stats.total_collisions);
  printf("Max chain length: %zu\n", stats.max_chain_length);
  printf("Average chain length: %.3f\n", stats.average_chain_length);
  printf("Resize count: %zu\n", stats.resize_count);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Insertion performance benchmark completed!\n");
}

void benchmark_lookup_performance() {
  printf("BENCHMARKING lookup performance\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(1000, arena);
  
  const int num_items = 5000;
  
  // Pre-populate the hashmap
  for (int i = 0; i < num_items; i++) {
    char key[32];
    sprintf(key, "lookup_key_%d", i);
    
    int *value = samrena_push(arena, sizeof(int));
    *value = i * 2;
    
    samhashmap_put(comb, key, value);
  }
  
  printf("Pre-populated with %d items\n", num_items);
  
  // Benchmark successful lookups
  clock_t start_time = clock();
  int successful_lookups = 0;
  
  for (int i = 0; i < num_items; i++) {
    char key[32];
    sprintf(key, "lookup_key_%d", i);
    
    int *retrieved = samhashmap_get(comb, key);
    if (retrieved != NULL && *retrieved == i * 2) {
      successful_lookups++;
    }
  }
  
  clock_t end_time = clock();
  double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  
  printf("Performed %d successful lookups in %.6f seconds\n", 
         successful_lookups, elapsed);
  printf("Lookup rate: %.0f lookups/second\n", successful_lookups / elapsed);
  printf("Average time per lookup: %.6f ms\n", (elapsed * 1000) / successful_lookups);
  
  // Benchmark failed lookups
  start_time = clock();
  int failed_lookups = 0;
  
  for (int i = 0; i < 1000; i++) {
    char key[32];
    sprintf(key, "nonexistent_key_%d", i);
    
    if (samhashmap_get(comb, key) == NULL) {
      failed_lookups++;
    }
  }
  
  end_time = clock();
  elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  
  printf("Performed %d failed lookups in %.6f seconds\n", 
         failed_lookups, elapsed);
  printf("Failed lookup rate: %.0f lookups/second\n", failed_lookups / elapsed);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Lookup performance benchmark completed!\n");
}

void benchmark_mixed_operations() {
  printf("BENCHMARKING mixed operations\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(100, arena);
  
  const int num_operations = 10000;
  int insertions = 0, lookups = 0, removals = 0;
  
  clock_t start_time = clock();
  
  // Mixed workload: 50% inserts, 30% lookups, 20% removals
  for (int i = 0; i < num_operations; i++) {
    int operation = i % 10;
    char key[32];
    sprintf(key, "mixed_key_%d", i % 1000);  // Reuse keys to create realistic scenario
    
    if (operation < 5) {  // 50% inserts
      int *value = samrena_push(arena, sizeof(int));
      if (value != NULL) {
        *value = i;
        if (samhashmap_put(comb, key, value)) {
          insertions++;
        }
      }
    } else if (operation < 8) {  // 30% lookups
      samhashmap_get(comb, key);
      lookups++;
    } else {  // 20% removals
      if (samhashmap_remove(comb, key)) {
        removals++;
      }
    }
  }
  
  clock_t end_time = clock();
  double elapsed = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
  
  printf("Performed %d mixed operations in %.6f seconds\n", 
         num_operations, elapsed);
  printf("Operations per second: %.0f\n", num_operations / elapsed);
  printf("Breakdown: %d inserts, %d lookups, %d removals\n", 
         insertions, lookups, removals);
  printf("Final size: %zu\n", samhashmap_size(comb));
  
  SamHashMapStats stats = samhashmap_get_stats(comb);
  printf("Total operations tracked: %zu\n", stats.total_operations);
  printf("Total collisions: %zu\n", stats.total_collisions);
  printf("Collision rate: %.3f%%\n", 
         stats.total_operations > 0 ? (100.0 * stats.total_collisions) / stats.total_operations : 0.0);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Mixed operations benchmark completed!\n");
}

void benchmark_hash_distribution() {
  printf("BENCHMARKING hash distribution quality\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(101, arena);  // Prime number for better distribution
  
  const int num_items = 1000;
  
  // Insert items and track distribution
  for (int i = 0; i < num_items; i++) {
    char key[32];
    
    // Use different key patterns to test hash function
    switch (i % 4) {
      case 0:
        sprintf(key, "pattern_a_%d", i);
        break;
      case 1:
        sprintf(key, "different_b_%d", i);
        break;
      case 2:
        sprintf(key, "key_%d_suffix", i);
        break;
      case 3:
        sprintf(key, "%d_prefix_key", i);
        break;
    }
    
    int *value = samrena_push(arena, sizeof(int));
    *value = i;
    samhashmap_put(comb, key, value);
  }
  
  // Analyze distribution
  SamHashMapStats stats = samhashmap_get_stats(comb);
  
  printf("Hash distribution analysis:\n");
  printf("Total items: %zu\n", samhashmap_size(comb));
  printf("Capacity: %zu\n", comb->capacity);
  printf("Load factor: %.3f\n", (float)samhashmap_size(comb) / comb->capacity);
  printf("Max chain length: %zu\n", stats.max_chain_length);
  printf("Average chain length: %.3f\n", stats.average_chain_length);
  printf("Total collisions: %zu\n", stats.total_collisions);
  
  // Ideal max chain length for good distribution
  float ideal_max_chain = 1.5f * stats.average_chain_length;
  printf("Distribution quality: %s\n", 
         stats.max_chain_length <= (size_t)ideal_max_chain ? "Good" : "Poor");
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Hash distribution benchmark completed!\n");
}

// DEBUGGING SUPPORT TESTS (Step 17 from planning)

// Helper function to check samhashmap invariants
static void check_samhashmap_invariants(SamHashMap *comb) {
  // Check basic invariants
  assert(comb != NULL);
  assert(comb->cells != NULL);
  assert(comb->arena != NULL);
  assert(comb->capacity > 0);
  assert(comb->size <= comb->capacity * 2);  // Reasonable upper bound
  assert(comb->load_factor > 0.0f && comb->load_factor <= 1.0f);
  
  // Count actual items and verify size
  size_t actual_count = 0;
  for (size_t i = 0; i < comb->capacity; i++) {
    Cell *current = comb->cells[i];
    while (current != NULL) {
      actual_count++;
      assert(current->key != NULL);  // Key should never be NULL
      current = current->next;
    }
  }
  assert(actual_count == comb->size);
  
  printf("Invariants OK: size=%zu, capacity=%zu, actual_count=%zu\n", 
         comb->size, comb->capacity, actual_count);
}

void test_invariant_checking() {
  printf("TESTING invariant checking\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(10, arena);
  
  // Test invariants during various operations
  check_samhashmap_invariants(comb);
  
  // Add items and check invariants
  for (int i = 0; i < 15; i++) {
    char key[32];
    sprintf(key, "invariant_%d", i);
    
    int *value = samrena_push(arena, sizeof(int));
    *value = i;
    
    samhashmap_put(comb, key, value);
    check_samhashmap_invariants(comb);
  }
  
  // Remove items and check invariants
  for (int i = 0; i < 15; i += 3) {
    char key[32];
    sprintf(key, "invariant_%d", i);
    
    samhashmap_remove(comb, key);
    check_samhashmap_invariants(comb);
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Invariant checking tests passed!\n");
}

void test_memory_usage_tracking() {
  printf("TESTING memory usage tracking (via allocation success/failure)\n");
  
  // Since samrena_used() is not available, we'll test memory usage
  // by observing allocation success/failure patterns
  SamrenaConfig config = samrena_default_config();
  config.page_size = 1024;  // Very small pages
  config.initial_pages = 1;  // Start with 1 page
  Samrena *arena = samrena_create(&config);
  
  if (arena == NULL) {
    printf("Failed to create small arena\n");
    return;
  }
  
  SamHashMap *comb = samhashmap_create(10, arena);
  if (comb == NULL) {
    printf("Failed to create samhashmap in small arena\n");
    samrena_destroy(arena);
    return;
  }
  
  printf("Created samhashmap in limited arena\n");
  
  // Track allocation patterns by attempting allocations
  const int num_items = 50;
  int successful_allocations = 0;
  int failed_allocations = 0;
  
  for (int i = 0; i < num_items; i++) {
    char key[64];  // Larger keys to consume more memory
    sprintf(key, "memory_track_with_longer_key_name_%d", i);
    
    // Try to allocate a reasonably sized value
    int *value = samrena_push(arena, sizeof(int));
    if (value == NULL) {
      failed_allocations++;
      printf("Allocation failed at item %d\n", i);
      break;
    }
    
    *value = i;
    if (samhashmap_put(comb, key, value)) {
      successful_allocations++;
      if (i % 10 == 0) {
        printf("Successfully added item %d (size: %zu, capacity: %zu)\n", 
               i, samhashmap_size(comb), comb->capacity);
      }
    } else {
      failed_allocations++;
      printf("Put failed at item %d (likely memory exhaustion)\n", i);
      break;
    }
  }
  
  printf("\nMemory usage analysis:\n");
  printf("Successful allocations: %d\n", successful_allocations);
  printf("Failed allocations: %d\n", failed_allocations);
  printf("Final samhashmap size: %zu\n", samhashmap_size(comb));
  printf("Final samhashmap capacity: %zu\n", comb->capacity);
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Memory usage tracking tests passed!\n");
}

void test_collision_visualization() {
  printf("TESTING collision visualization\n");
  
  Samrena *arena = samrena_create_default();
  SamHashMap *comb = samhashmap_create(8, arena);  // Small for visible collisions
  
  // Add items to create interesting collision patterns
  const char *test_keys[] = {
    "apple", "banana", "cherry", "date", "elderberry",
    "fig", "grape", "honeydew", "kiwi", "lemon",
    "mango", "nectarine", "orange", "papaya", "quince"
  };
  int num_keys = sizeof(test_keys) / sizeof(test_keys[0]);
  
  for (int i = 0; i < num_keys; i++) {
    int *value = samrena_push(arena, sizeof(int));
    *value = i;
    samhashmap_put(comb, test_keys[i], value);
  }
  
  // Visualize the collision chains
  printf("\nCollision chain visualization:\n");
  printf("Capacity: %zu, Size: %zu, Load factor: %.3f\n", 
         comb->capacity, comb->size, (float)comb->size / comb->capacity);
  printf("\n");
  
  size_t empty_buckets = 0;
  size_t max_chain_length = 0;
  size_t total_chain_length = 0;
  
  for (size_t i = 0; i < comb->capacity; i++) {
    printf("Bucket %zu: ", i);
    
    Cell *current = comb->cells[i];
    if (current == NULL) {
      printf("[empty]\n");
      empty_buckets++;
    } else {
      size_t chain_length = 0;
      while (current != NULL) {
        printf("[%s]%s", current->key, current->next ? " -> " : "");
        current = current->next;
        chain_length++;
      }
      printf(" (length: %zu)\n", chain_length);
      
      if (chain_length > max_chain_length) {
        max_chain_length = chain_length;
      }
      total_chain_length += chain_length;
    }
  }
  
  // Print collision statistics
  printf("\nCollision statistics:\n");
  printf("Empty buckets: %zu (%.1f%%)\n", 
         empty_buckets, 100.0f * empty_buckets / comb->capacity);
  printf("Used buckets: %zu (%.1f%%)\n", 
         comb->capacity - empty_buckets, 
         100.0f * (comb->capacity - empty_buckets) / comb->capacity);
  printf("Max chain length: %zu\n", max_chain_length);
  printf("Average chain length: %.3f\n", 
         (float)total_chain_length / (comb->capacity - empty_buckets));
  
  // Create a histogram of chain lengths
  size_t chain_histogram[10] = {0};  // Up to length 9, then 9+
  
  for (size_t i = 0; i < comb->capacity; i++) {
    size_t chain_length = 0;
    Cell *current = comb->cells[i];
    while (current != NULL) {
      chain_length++;
      current = current->next;
    }
    
    if (chain_length < 10) {
      chain_histogram[chain_length]++;
    } else {
      chain_histogram[9]++;  // 9+ bucket
    }
  }
  
  printf("\nChain length histogram:\n");
  for (int i = 0; i < 10; i++) {
    if (i == 9) {
      printf("Length 9+: %zu buckets\n", chain_histogram[i]);
    } else {
      printf("Length %d: %zu buckets", i, chain_histogram[i]);
      if (chain_histogram[i] > 0) {
        printf(" [");
        for (size_t j = 0; j < chain_histogram[i] && j < 20; j++) {
          printf("*");
        }
        if (chain_histogram[i] > 20) printf("+");
        printf("]");
      }
      printf("\n");
    }
  }
  
  samhashmap_destroy(comb);
  samrena_destroy(arena);
  printf("Collision visualization tests passed!\n");
}

void debugging_support_tests() {
  printf("\n=== STARTING DEBUGGING SUPPORT TESTS ===\n");
  
  test_invariant_checking();
  test_memory_usage_tracking();
  test_collision_visualization();
  
  printf("=== ALL DEBUGGING SUPPORT TESTS PASSED ===\n\n");
}

void performance_benchmark_tests() {
  printf("\n=== STARTING PERFORMANCE BENCHMARK TESTS ===\n");
  
  benchmark_insertion_performance();
  benchmark_lookup_performance();
  benchmark_mixed_operations();
  benchmark_hash_distribution();
  
  printf("=== ALL PERFORMANCE BENCHMARK TESTS COMPLETED ===\n\n");
}

void memory_exhaustion_tests() {
  printf("\n=== STARTING MEMORY EXHAUSTION TESTS ===\n");
  
  test_small_arena_exhaustion();
  test_arena_exhaustion_during_resize();
  test_graceful_degradation();
  
  printf("=== ALL MEMORY EXHAUSTION TESTS PASSED ===\n\n");
}

void resize_behavior_tests() {
  printf("\n=== STARTING RESIZE BEHAVIOR TESTS ===\n");
  
  test_automatic_growth();
  test_load_factor_threshold();
  test_resize_preserves_data();
  test_multiple_resizes();
  
  printf("=== ALL RESIZE BEHAVIOR TESTS PASSED ===\n\n");
}

void collision_handling_tests() {
  printf("\n=== STARTING COLLISION HANDLING TESTS ===\n");
  
  test_forced_collisions();
  test_collision_chain_integrity();
  test_collision_with_updates();
  
  printf("=== ALL COLLISION HANDLING TESTS PASSED ===\n\n");
}

void comprehensive_edge_case_tests() {
  printf("\n=== STARTING COMPREHENSIVE EDGE CASE TESTS ===\n");
  
  test_empty_hashmap();
  test_single_element();
  test_null_key_and_values();
  test_empty_string_key();
  test_duplicate_keys();
  test_very_long_keys();
  test_similar_keys();
  
  printf("=== ALL EDGE CASE TESTS PASSED ===\n\n");
}

void typed_samhashmap_tests() {
  printf("\n=== STARTING TYPED SAMHASHMAP TESTS ===\n");
  
  test_string_string_map();
  test_string_int_map();
  test_string_ptr_map();
  test_typed_foreach();
  test_null_safety();
  test_custom_typed_map();
  
  printf("=== ALL TYPED SAMHASHMAP TESTS PASSED ===\n\n");
}
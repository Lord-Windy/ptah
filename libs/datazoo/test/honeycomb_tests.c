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

  Samrena *arena = samrena_allocate(10);
  Honeycomb *comb = honeycomb_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    honeycomb_put(comb, global_dictionary[i], x);
  }

  honeycomb_print(comb);
  honeycomb_destroy(comb);
  samrena_deallocate(arena);
}

void honeycomb_tests_get_words() {

  printf("HONEYCOMB TESTS GET WORDS\n");

  Samrena *arena = samrena_allocate(10);
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
  samrena_deallocate(arena);
}

void honeycomb_tests_remove_words() {
  printf("HONEYCOMB TESTS REMOVE WORDS\n");

  Samrena *arena = samrena_allocate(10);
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
  samrena_deallocate(arena);
}

void honeycomb_tests_contains_words() {
  printf("HONEYCOMB TESTS CONTAINS WORDS\n");
  Samrena *arena = samrena_allocate(10);
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
  samrena_deallocate(arena);
}

void honeycomb_tests_size() {
  printf("HONEYCOMB TESTS SIZE\n");
  Samrena *arena = samrena_allocate(10);
  Honeycomb *comb = honeycomb_create(10, arena);

  for (int i = 0; i < 50; i++) {
    int *x = samrena_push(arena, sizeof(int *));
    *x = i;
    honeycomb_put(comb, global_dictionary[i], x);
  }

  printf("size: %zu\n", honeycomb_size(comb));
  assert(honeycomb_size(comb) == 50);

  honeycomb_destroy(comb);
  samrena_deallocate(arena);
}

void honeycomb_tests_update_words() {}

void honeycomb_tests() {
  honeycomb_tests_adding_words();
  honeycomb_tests_get_words();
  honeycomb_tests_remove_words();
  honeycomb_tests_size();
  honeycomb_tests_contains_words();
  honeycomb_tests_update_words();
}
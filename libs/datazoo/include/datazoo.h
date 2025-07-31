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

#ifndef DATAZOO_H
#define DATAZOO_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <samrena.h>

// HONEYCOMB
//  Define the structure for each key-value pair
typedef struct Cell {
  char *key;
  void *value;
  struct Cell *next;
} Cell;

// Define the main hashmap structure
typedef struct {
  Cell **cells;
  size_t size;     // Current number of elements
  size_t capacity; // Number of buckets
  Samrena *arena; // Arena for memory allocation
  float load_factor; // Threshold for resizing (default 0.75)
} Honeycomb;

// Function declarations

/**
 * Creates a new honeycomb hash map.
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @return New honeycomb instance or NULL if samrena is NULL
 */
Honeycomb *honeycomb_create(size_t initial_capacity, Samrena *samrena);

void honeycomb_destroy(Honeycomb *comb);

bool honeycomb_put(Honeycomb *comb, const char *key, void *value);
void *honeycomb_get(const Honeycomb *comb, const char *key);

bool honeycomb_remove(Honeycomb *comb, const char *key);
bool honeycomb_contains(const Honeycomb *comb, const char *key);

void honeycomb_clear(Honeycomb *comb);
void honeycomb_print(const Honeycomb *comb);

size_t honeycomb_size(const Honeycomb *comb);
bool honeycomb_is_empty(const Honeycomb *comb);

// Collection functions
size_t honeycomb_get_keys(const Honeycomb *comb, const char **keys, size_t max_keys);
size_t honeycomb_get_values(const Honeycomb *comb, void **values, size_t max_values);

// Optional: Iterator functions
typedef void (*HoneycombIterator)(const char *key, void *value, void *user_data);
void honeycomb_foreach(const Honeycomb *map, HoneycombIterator iterator, void *user_data);

#endif // DATAZOO_H

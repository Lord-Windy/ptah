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

#include <stdio.h>
#include <string.h>

#include "datazoo.h"
#include "samrena.h"

#define DEFAULT_PAGE_COUNT 64

Cell *honeycomb_cell_create(Samrena *arena, const char *key, void *value) {
  Cell *cell = samrena_push(arena, sizeof(Cell));
  if (cell == NULL) {
    return NULL;
  }

  // Copy the key into arena-allocated memory
  size_t key_len = strlen(key) + 1;
  char *key_copy = (char *)samrena_push(arena, key_len);
  if (key_copy == NULL) {
    return NULL;
  }
  memcpy(key_copy, key, key_len);

  cell->key = key_copy;
  cell->value = value;
  cell->next = NULL;

  return cell;
}

Honeycomb *honeycomb_create(size_t initial_capacity, Samrena *samrena) {
  if (samrena == NULL) {
    return NULL;  // Samrena instance is required
  }

  Honeycomb *comb = samrena_push(samrena, sizeof(Honeycomb));
  if (comb == NULL) {
    return NULL;
  }

  comb->cells = samrena_push_zero(samrena, sizeof(Cell *) * initial_capacity);
  if (comb->cells == NULL) {
    return NULL;
  }
  
  comb->size = 0;
  comb->capacity = initial_capacity;
  comb->arena = samrena;
  comb->load_factor = 0.75f;

  return comb;
}

void honeycomb_destroy(Honeycomb *comb) {
  // Memory is managed by the caller through samrena
  // No cleanup needed here
}

static size_t hash_djb2(const char *key, size_t capacity) {
  size_t hash = 5381;
  int c;
  while ((c = *key++)) {
    hash = (hash << 5) + hash + c; /* hash * 33 + c */
  }
  return hash % capacity;
}

static bool honeycomb_resize(Honeycomb *comb) {
  size_t new_capacity = comb->capacity * 2;
  Cell **new_cells = samrena_push_zero(comb->arena, sizeof(Cell *) * new_capacity);
  if (new_cells == NULL) {
    return false; // Arena exhausted
  }

  // Rehash all existing entries
  for (size_t i = 0; i < comb->capacity; i++) {
    Cell *current = comb->cells[i];
    while (current != NULL) {
      Cell *next = current->next;
      
      // Rehash to new bucket
      size_t new_bucket = hash_djb2(current->key, new_capacity);
      current->next = new_cells[new_bucket];
      new_cells[new_bucket] = current;
      
      current = next;
    }
  }

  // Update the honeycomb structure
  comb->cells = new_cells;
  comb->capacity = new_capacity;
  
  return true;
}

bool honeycomb_put(Honeycomb *comb, const char *key, void *value) {
  // Validate parameters
  if (comb == NULL || key == NULL) {
    return false;
  }
  // Check if resize is needed
  if (comb->size >= comb->capacity * comb->load_factor) {
    if (!honeycomb_resize(comb)) {
      // Resize failed - continue anyway, performance will degrade
    }
  }

  size_t hash_bucket = hash_djb2(key, comb->capacity);
  
  // First, check if key already exists to avoid unnecessary allocation
  Cell *current = comb->cells[hash_bucket];
  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      current->value = value;
      return true;
    }
    current = current->next;
  }

  // Key doesn't exist, create new cell
  Cell *new_cell = honeycomb_cell_create(comb->arena, key, value);
  if (new_cell == NULL) {
    // Failed to allocate memory for new cell
    return false;
  }
  
  // Insert at the beginning of the bucket chain
  new_cell->next = comb->cells[hash_bucket];
  comb->cells[hash_bucket] = new_cell;
  comb->size++;
  return true;
}

void *honeycomb_get(const Honeycomb *comb, const char *key) {
  // Validate parameters
  if (comb == NULL || key == NULL) {
    return NULL;
  }
  size_t hash_bucket = hash_djb2(key, comb->capacity);
  Cell *current = comb->cells[hash_bucket];
  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      return current->value;
    }
    current = current->next;
  }
  return NULL;
}

bool honeycomb_remove(Honeycomb *comb, const char *key) {
  // Validate parameters
  if (comb == NULL || key == NULL) {
    return false;
  }
  size_t hash_bucket = hash_djb2(key, comb->capacity);
  Cell *current = comb->cells[hash_bucket];
  Cell *previous = NULL;
  while (current != NULL) {

    if (strcmp(current->key, key) == 0) {
      // If the cell is the first cell in the bucket
      if (previous == NULL) {
        comb->cells[hash_bucket] = current->next;
      } else {
        previous->next = current->next;
      }
      comb->size--;
      return true;
    }

    previous = current;
    current = current->next;
  }
  return false; // Key not found
}

bool honeycomb_contains(const Honeycomb *comb, const char *key) {
  // Validate parameters
  if (comb == NULL || key == NULL) {
    return false;
  }
  size_t hash_bucket = hash_djb2(key, comb->capacity);
  Cell *current = comb->cells[hash_bucket];
  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      return true;
    }
    current = current->next;
  }
  return false;
}

void honeycomb_clear(Honeycomb *comb) {
  if (comb == NULL) {
    return;
  }
  
  // Zero out all buckets
  memset(comb->cells, 0, sizeof(Cell *) * comb->capacity);
  comb->size = 0;
  // Note: Memory remains allocated in arena - cannot be freed
}

void honeycomb_print(const Honeycomb *comb) {
  if (comb == NULL) {
    return;
  }
  for (size_t i = 0; i < comb->capacity; i++) {
    Cell *current = comb->cells[i];
    while (current != NULL) {
      printf("%s: %p\n", current->key, current->value);
      current = current->next;
    }
  }
}

size_t honeycomb_size(const Honeycomb *comb) {
  if (comb == NULL) {
    return 0;
  }
  return comb->size;
}

bool honeycomb_is_empty(const Honeycomb *comb) {
  if (comb == NULL) {
    return true;
  }
  return comb->size == 0;
}

size_t honeycomb_get_keys(const Honeycomb *comb, const char **keys, size_t max_keys) {
  if (comb == NULL || keys == NULL || max_keys == 0) {
    return 0;
  }
  
  size_t count = 0;
  for (size_t i = 0; i < comb->capacity && count < max_keys; i++) {
    Cell *current = comb->cells[i];
    while (current != NULL && count < max_keys) {
      keys[count] = current->key;
      count++;
      current = current->next;
    }
  }
  
  return count;
}

size_t honeycomb_get_values(const Honeycomb *comb, void **values, size_t max_values) {
  if (comb == NULL || values == NULL || max_values == 0) {
    return 0;
  }
  
  size_t count = 0;
  for (size_t i = 0; i < comb->capacity && count < max_values; i++) {
    Cell *current = comb->cells[i];
    while (current != NULL && count < max_values) {
      values[count] = current->value;
      count++;
      current = current->next;
    }
  }
  
  return count;
}

void honeycomb_foreach(const Honeycomb *map, HoneycombIterator iterator, void *user_data) {
  if (map == NULL || iterator == NULL) {
    return;
  }
  
  for (size_t i = 0; i < map->capacity; i++) {
    Cell *current = map->cells[i];
    while (current != NULL) {
      iterator(current->key, current->value, user_data);
      current = current->next;
    }
  }
}

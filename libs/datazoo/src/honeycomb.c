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

Cell *honeycomb_cell_create(Samrena *arena, char *key, void *value) {
  Cell *cell = samrena_push(arena, sizeof(Cell));

  cell->key = key;
  cell->value = value;
  cell->next = NULL;

  return cell;
}

Honeycomb *honeycomb_create(size_t initial_capacity, Samrena *samrena) {

  Samrena *arena = samrena != NULL ? samrena : samrena_allocate(64);

  Honeycomb *comb = samrena_push(arena, sizeof(Honeycomb));

  comb->cells = samrena_push_zero(arena, sizeof(Cell *) * initial_capacity);
  comb->size = 0;
  comb->capacity = initial_capacity;
  comb->arena = arena;
  comb->memory_managed_internally = samrena == NULL;

  return comb;
}

void honeycomb_destroy(Honeycomb *comb) {
  if (comb->memory_managed_internally) {
    samrena_deallocate(comb->arena);
  }
}

size_t hash_djb2(const char *key, size_t capacity) {
  size_t hash = 5381;
  int c;
  while ((c = *key++)) {
    hash = (hash << 5) + hash + c; /* hash * 33 + c */
  }
  return hash % capacity;
}

void honeycomb_put(Honeycomb *comb, const char *key, void *value) {

  size_t hash_bucket = hash_djb2(key, comb->capacity);

  Cell *new_cell = honeycomb_cell_create(comb->arena, (char *)key, value);
  Cell *current = comb->cells[hash_bucket];

  if (current == NULL) {
    comb->cells[hash_bucket] = new_cell;
    comb->size++;
    return;
  }

  // Traverse to the end of the linked list in the bucket
  while (current->next != NULL) {
    // Update value if the key already exists
    if (strcmp(current->key, key) == 0) {
      current->value = value;
      return;
    }
    current = current->next;
  }

  // Check the last node for key match
  if (strcmp(current->key, key) == 0) {
    current->value = value;
  } else {
    // Insert the new cell at the end of the list
    current->next = new_cell;
    comb->size++;
  }
}

void *honeycomb_get(Honeycomb *comb, const char *key) {
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

void honeycomb_remove(Honeycomb *comb, const char *key) {
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
      return;
    }

    previous = current;
    current = current->next;
  }
}

bool honeycomb_contains(Honeycomb *comb, const char *key) {
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

void honeycomb_print(Honeycomb *comb) {
  for (size_t i = 0; i < comb->capacity; i++) {
    Cell *current = comb->cells[i];
    while (current != NULL) {
      printf("%s: %p\n", current->key, current->value);
      current = current->next;
    }
  }
}

size_t honeycomb_size(const Honeycomb *comb) { return comb->size; }

bool honeycomb_is_empty(const Honeycomb *comb) { return comb->size == 0; }

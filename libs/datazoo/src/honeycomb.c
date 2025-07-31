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
#include "datazoo/hash.h"
#include "samrena.h"

#define DEFAULT_PAGE_COUNT 64

static void honeycomb_report_error(Honeycomb *comb, HoneycombError error, const char *message) {
  if (comb == NULL) return;
  
  comb->last_error = error;
  if (comb->error_callback != NULL) {
    comb->error_callback(error, message, comb->error_callback_data);
  }
}

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
  return honeycomb_create_with_hash(initial_capacity, samrena, HASH_DJB2);
}

Honeycomb *honeycomb_create_with_hash(size_t initial_capacity, Samrena *samrena, HashFunction hash_func) {
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
  comb->hash_func = hash_func;
  comb->error_callback = NULL;
  comb->error_callback_data = NULL;
  comb->last_error = HONEYCOMB_ERROR_NONE;
  
  // Initialize stats
  memset(&comb->stats, 0, sizeof(HoneycombStats));

  return comb;
}

void honeycomb_destroy(Honeycomb *comb) {
  // Memory is managed by the caller through samrena
  // No cleanup needed here
}


static size_t hash_function(const char *key, size_t capacity, HashFunction func) {
  DatazooHashFunction datazoo_func;
  switch (func) {
    case HASH_DJB2:
      datazoo_func = DATAZOO_HASH_DJB2;
      break;
    case HASH_FNV1A:
      datazoo_func = DATAZOO_HASH_FNV1A;
      break;
    case HASH_MURMUR3:
      datazoo_func = DATAZOO_HASH_MURMUR3;
      break;
    default:
      datazoo_func = DATAZOO_HASH_DJB2;
      break;
  }
  return datazoo_hash_string(key, datazoo_func) % capacity;
}

static bool honeycomb_resize(Honeycomb *comb) {
  size_t new_capacity = comb->capacity * 2;
  Cell **new_cells = samrena_push_zero(comb->arena, sizeof(Cell *) * new_capacity);
  if (new_cells == NULL) {
    comb->stats.failed_allocations++;
    honeycomb_report_error(comb, HONEYCOMB_ERROR_MEMORY_EXHAUSTED, 
                          "Failed to allocate memory for hashmap resize");
    return false; // Arena exhausted
  }

  // Rehash all existing entries
  for (size_t i = 0; i < comb->capacity; i++) {
    Cell *current = comb->cells[i];
    while (current != NULL) {
      Cell *next = current->next;
      
      // Rehash to new bucket
      size_t new_bucket = hash_function(current->key, new_capacity, comb->hash_func);
      current->next = new_cells[new_bucket];
      new_cells[new_bucket] = current;
      
      current = next;
    }
  }

  // Update the honeycomb structure
  comb->cells = new_cells;
  comb->capacity = new_capacity;
  comb->stats.resize_count++;
  
  return true;
}

bool honeycomb_put(Honeycomb *comb, const char *key, void *value) {
  // Validate parameters
  if (comb == NULL || key == NULL) {
    if (comb != NULL) {
      honeycomb_report_error(comb, HONEYCOMB_ERROR_NULL_PARAM, "Null parameter passed to honeycomb_put");
    }
    return false;
  }
  // Check if resize is needed
  if (comb->size >= comb->capacity * comb->load_factor) {
    if (!honeycomb_resize(comb)) {
      // Resize failed - continue anyway, performance will degrade
      honeycomb_report_error(comb, HONEYCOMB_ERROR_RESIZE_FAILED, 
                            "Hashmap resize failed, performance may degrade");
    }
  }

  size_t hash_bucket = hash_function(key, comb->capacity, comb->hash_func);
  comb->stats.total_operations++;
  
  // First, check if key already exists to avoid unnecessary allocation
  Cell *current = comb->cells[hash_bucket];
  size_t chain_length = 0;
  while (current != NULL) {
    chain_length++;
    if (strcmp(current->key, key) == 0) {
      current->value = value;
      return true;
    }
    current = current->next;
  }
  
  // Track collision if chain_length > 0
  if (chain_length > 0) {
    comb->stats.total_collisions++;
  }
  if (chain_length > comb->stats.max_chain_length) {
    comb->stats.max_chain_length = chain_length;
  }

  // Key doesn't exist, create new cell
  Cell *new_cell = honeycomb_cell_create(comb->arena, key, value);
  if (new_cell == NULL) {
    // Failed to allocate memory for new cell
    comb->stats.failed_allocations++;
    honeycomb_report_error(comb, HONEYCOMB_ERROR_MEMORY_EXHAUSTED, 
                          "Failed to allocate memory for new cell");
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
  size_t hash_bucket = hash_function(key, comb->capacity, comb->hash_func);
  // Note: Can't update stats in const function
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
  size_t hash_bucket = hash_function(key, comb->capacity, comb->hash_func);
  comb->stats.total_operations++;
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
  size_t hash_bucket = hash_function(key, comb->capacity, comb->hash_func);
  // Note: Can't update stats in const function
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

HoneycombStats honeycomb_get_stats(const Honeycomb *comb) {
  if (comb == NULL) {
    HoneycombStats empty_stats;
    memset(&empty_stats, 0, sizeof(HoneycombStats));
    return empty_stats;
  }
  
  HoneycombStats stats = comb->stats;
  
  // Calculate average chain length
  if (comb->capacity > 0) {
    size_t total_chain_length = 0;
    size_t non_empty_buckets = 0;
    
    for (size_t i = 0; i < comb->capacity; i++) {
      size_t chain_length = 0;
      Cell *current = comb->cells[i];
      while (current != NULL) {
        chain_length++;
        current = current->next;
      }
      if (chain_length > 0) {
        total_chain_length += chain_length;
        non_empty_buckets++;
      }
    }
    
    if (non_empty_buckets > 0) {
      stats.average_chain_length = (double)total_chain_length / non_empty_buckets;
    }
  }
  
  return stats;
}

void honeycomb_reset_stats(Honeycomb *comb) {
  if (comb == NULL) {
    return;
  }
  memset(&comb->stats, 0, sizeof(HoneycombStats));
}

void honeycomb_print_stats(const Honeycomb *comb) {
  if (comb == NULL) {
    printf("Honeycomb is NULL\n");
    return;
  }
  
  HoneycombStats stats = honeycomb_get_stats(comb);
  
  printf("Honeycomb Statistics:\n");
  printf("  Size: %zu\n", comb->size);
  printf("  Capacity: %zu\n", comb->capacity);
  printf("  Load Factor: %.2f\n", comb->load_factor);
  printf("  Hash Function: %s\n", 
         comb->hash_func == HASH_DJB2 ? "DJB2" :
         comb->hash_func == HASH_FNV1A ? "FNV1A" :
         comb->hash_func == HASH_MURMUR3 ? "MurmurHash3" : "Unknown");
  printf("  Total Operations: %zu\n", stats.total_operations);
  printf("  Total Collisions: %zu\n", stats.total_collisions);
  printf("  Max Chain Length: %zu\n", stats.max_chain_length);
  printf("  Average Chain Length: %.2f\n", stats.average_chain_length);
  printf("  Resize Count: %zu\n", stats.resize_count);
  printf("  Failed Allocations: %zu\n", stats.failed_allocations);
  if (stats.total_operations > 0) {
    printf("  Collision Rate: %.2f%%\n", 
           (double)stats.total_collisions / stats.total_operations * 100.0);
  }
}

void honeycomb_set_error_callback(Honeycomb *comb, HoneycombErrorCallback callback, void *user_data) {
  if (comb == NULL) {
    return;
  }
  comb->error_callback = callback;
  comb->error_callback_data = user_data;
}

HoneycombError honeycomb_get_last_error(const Honeycomb *comb) {
  if (comb == NULL) {
    return HONEYCOMB_ERROR_NULL_PARAM;
  }
  return comb->last_error;
}

const char *honeycomb_error_string(HoneycombError error) {
  switch (error) {
    case HONEYCOMB_ERROR_NONE:
      return "No error";
    case HONEYCOMB_ERROR_NULL_PARAM:
      return "Null parameter";
    case HONEYCOMB_ERROR_MEMORY_EXHAUSTED:
      return "Memory exhausted";
    case HONEYCOMB_ERROR_RESIZE_FAILED:
      return "Resize operation failed";
    case HONEYCOMB_ERROR_KEY_NOT_FOUND:
      return "Key not found";
    default:
      return "Unknown error";
  }
}

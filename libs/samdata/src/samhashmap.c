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

#include "samdata.h"
#include "samdata/samhash.h"
#include "samrena.h"

#define DEFAULT_PAGE_COUNT 64

static void samhashmap_report_error(SamHashMap *map, SamHashMapError error, const char *message) {
  if (map == NULL)
    return;

  map->last_error = error;
  if (map->error_callback != NULL) {
    map->error_callback(error, message, map->error_callback_data);
  }
}

Cell *samhashmap_cell_create(Samrena *arena, const char *key, void *value) {
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

SamHashMap *samhashmap_create(size_t initial_capacity, Samrena *samrena) {
  return samhashmap_create_with_hash(initial_capacity, samrena, SAMHASHMAP_HASH_DJB2);
}

SamHashMap *samhashmap_create_with_hash(size_t initial_capacity, Samrena *samrena,
                                        SamHashMapHashFunction hash_func) {
  if (samrena == NULL) {
    return NULL; // Samrena instance is required
  }

  SamHashMap *map = samrena_push(samrena, sizeof(SamHashMap));
  if (map == NULL) {
    return NULL;
  }

  map->cells = samrena_push_zero(samrena, sizeof(Cell *) * initial_capacity);
  if (map->cells == NULL) {
    return NULL;
  }

  map->size = 0;
  map->capacity = initial_capacity;
  map->arena = samrena;
  map->load_factor = 0.75f;
  map->hash_func = hash_func;
  map->error_callback = NULL;
  map->error_callback_data = NULL;
  map->last_error = SAMHASHMAP_ERROR_NONE;

  // Initialize stats
  memset(&map->stats, 0, sizeof(SamHashMapStats));

  return map;
}

void samhashmap_destroy(SamHashMap *map) {
  // Memory is managed by the caller through samrena
  // No cleanup needed here
}

static size_t hash_function(const char *key, size_t capacity, SamHashMapHashFunction func) {
  SamHashFunction samhash_func;
  switch (func) {
    case SAMHASHMAP_HASH_DJB2:
      samhash_func = SAMHASH_DJB2;
      break;
    case SAMHASHMAP_HASH_FNV1A:
      samhash_func = SAMHASH_FNV1A;
      break;
    case SAMHASHMAP_HASH_MURMUR3:
      samhash_func = SAMHASH_MURMUR3;
      break;
    default:
      samhash_func = SAMHASH_DJB2;
      break;
  }
  return samhash_string(key, samhash_func) % capacity;
}

static bool samhashmap_resize(SamHashMap *map) {
  size_t new_capacity = map->capacity * 2;
  Cell **new_cells = samrena_push_zero(map->arena, sizeof(Cell *) * new_capacity);
  if (new_cells == NULL) {
    map->stats.failed_allocations++;
    samhashmap_report_error(map, SAMHASHMAP_ERROR_MEMORY_EXHAUSTED,
                            "Failed to allocate memory for hashmap resize");
    return false; // Arena exhausted
  }

  // Rehash all existing entries
  for (size_t i = 0; i < map->capacity; i++) {
    Cell *current = map->cells[i];
    while (current != NULL) {
      Cell *next = current->next;

      // Rehash to new bucket
      size_t new_bucket = hash_function(current->key, new_capacity, map->hash_func);
      current->next = new_cells[new_bucket];
      new_cells[new_bucket] = current;

      current = next;
    }
  }

  // Update the samhashmap structure
  map->cells = new_cells;
  map->capacity = new_capacity;
  map->stats.resize_count++;

  return true;
}

bool samhashmap_put(SamHashMap *map, const char *key, void *value) {
  // Validate parameters
  if (map == NULL || key == NULL) {
    if (map != NULL) {
      samhashmap_report_error(map, SAMHASHMAP_ERROR_NULL_PARAM,
                              "Null parameter passed to samhashmap_put");
    }
    return false;
  }
  // Check if resize is needed
  if (map->size >= map->capacity * map->load_factor) {
    if (!samhashmap_resize(map)) {
      // Resize failed - continue anyway, performance will degrade
      samhashmap_report_error(map, SAMHASHMAP_ERROR_RESIZE_FAILED,
                              "Hashmap resize failed, performance may degrade");
    }
  }

  size_t hash_bucket = hash_function(key, map->capacity, map->hash_func);
  map->stats.total_operations++;

  // First, check if key already exists to avoid unnecessary allocation
  Cell *current = map->cells[hash_bucket];
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
    map->stats.total_collisions++;
  }
  if (chain_length > map->stats.max_chain_length) {
    map->stats.max_chain_length = chain_length;
  }

  // Key doesn't exist, create new cell
  Cell *new_cell = samhashmap_cell_create(map->arena, key, value);
  if (new_cell == NULL) {
    // Failed to allocate memory for new cell
    map->stats.failed_allocations++;
    samhashmap_report_error(map, SAMHASHMAP_ERROR_MEMORY_EXHAUSTED,
                            "Failed to allocate memory for new cell");
    return false;
  }

  // Insert at the beginning of the bucket chain
  new_cell->next = map->cells[hash_bucket];
  map->cells[hash_bucket] = new_cell;
  map->size++;
  return true;
}

void *samhashmap_get(const SamHashMap *map, const char *key) {
  // Validate parameters
  if (map == NULL || key == NULL) {
    return NULL;
  }
  size_t hash_bucket = hash_function(key, map->capacity, map->hash_func);
  // Note: Can't update stats in const function
  Cell *current = map->cells[hash_bucket];
  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      return current->value;
    }
    current = current->next;
  }
  return NULL;
}

bool samhashmap_remove(SamHashMap *map, const char *key) {
  // Validate parameters
  if (map == NULL || key == NULL) {
    return false;
  }
  size_t hash_bucket = hash_function(key, map->capacity, map->hash_func);
  map->stats.total_operations++;
  Cell *current = map->cells[hash_bucket];
  Cell *previous = NULL;
  while (current != NULL) {

    if (strcmp(current->key, key) == 0) {
      // If the cell is the first cell in the bucket
      if (previous == NULL) {
        map->cells[hash_bucket] = current->next;
      } else {
        previous->next = current->next;
      }
      map->size--;
      return true;
    }

    previous = current;
    current = current->next;
  }
  return false; // Key not found
}

bool samhashmap_contains(const SamHashMap *map, const char *key) {
  // Validate parameters
  if (map == NULL || key == NULL) {
    return false;
  }
  size_t hash_bucket = hash_function(key, map->capacity, map->hash_func);
  // Note: Can't update stats in const function
  Cell *current = map->cells[hash_bucket];
  while (current != NULL) {
    if (strcmp(current->key, key) == 0) {
      return true;
    }
    current = current->next;
  }
  return false;
}

void samhashmap_clear(SamHashMap *map) {
  if (map == NULL) {
    return;
  }

  // Zero out all buckets
  memset(map->cells, 0, sizeof(Cell *) * map->capacity);
  map->size = 0;
  // Note: Memory remains allocated in arena - cannot be freed
}

void samhashmap_print(const SamHashMap *map) {
  if (map == NULL) {
    return;
  }
  for (size_t i = 0; i < map->capacity; i++) {
    Cell *current = map->cells[i];
    while (current != NULL) {
      printf("%s: %p\n", current->key, current->value);
      current = current->next;
    }
  }
}

size_t samhashmap_size(const SamHashMap *map) {
  if (map == NULL) {
    return 0;
  }
  return map->size;
}

bool samhashmap_is_empty(const SamHashMap *map) {
  if (map == NULL) {
    return true;
  }
  return map->size == 0;
}

size_t samhashmap_get_keys(const SamHashMap *map, const char **keys, size_t max_keys) {
  if (map == NULL || keys == NULL || max_keys == 0) {
    return 0;
  }

  size_t count = 0;
  for (size_t i = 0; i < map->capacity && count < max_keys; i++) {
    Cell *current = map->cells[i];
    while (current != NULL && count < max_keys) {
      keys[count] = current->key;
      count++;
      current = current->next;
    }
  }

  return count;
}

size_t samhashmap_get_values(const SamHashMap *map, void **values, size_t max_values) {
  if (map == NULL || values == NULL || max_values == 0) {
    return 0;
  }

  size_t count = 0;
  for (size_t i = 0; i < map->capacity && count < max_values; i++) {
    Cell *current = map->cells[i];
    while (current != NULL && count < max_values) {
      values[count] = current->value;
      count++;
      current = current->next;
    }
  }

  return count;
}

void samhashmap_foreach(const SamHashMap *map, SamHashMapIterator iterator, void *user_data) {
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

SamHashMapStats samhashmap_get_stats(const SamHashMap *map) {
  if (map == NULL) {
    SamHashMapStats empty_stats;
    memset(&empty_stats, 0, sizeof(SamHashMapStats));
    return empty_stats;
  }

  SamHashMapStats stats = map->stats;

  // Calculate average chain length
  if (map->capacity > 0) {
    size_t total_chain_length = 0;
    size_t non_empty_buckets = 0;

    for (size_t i = 0; i < map->capacity; i++) {
      size_t chain_length = 0;
      Cell *current = map->cells[i];
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

void samhashmap_reset_stats(SamHashMap *map) {
  if (map == NULL) {
    return;
  }
  memset(&map->stats, 0, sizeof(SamHashMapStats));
}

void samhashmap_print_stats(const SamHashMap *map) {
  if (map == NULL) {
    printf("SamHashMap is NULL\n");
    return;
  }

  SamHashMapStats stats = samhashmap_get_stats(map);

  printf("SamHashMap Statistics:\n");
  printf("  Size: %zu\n", map->size);
  printf("  Capacity: %zu\n", map->capacity);
  printf("  Load Factor: %.2f\n", map->load_factor);
  printf("  Hash Function: %s\n", map->hash_func == SAMHASHMAP_HASH_DJB2      ? "DJB2"
                                  : map->hash_func == SAMHASHMAP_HASH_FNV1A   ? "FNV1A"
                                  : map->hash_func == SAMHASHMAP_HASH_MURMUR3 ? "MurmurHash3"
                                                                              : "Unknown");
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

void samhashmap_set_error_callback(SamHashMap *map, SamHashMapErrorCallback callback,
                                   void *user_data) {
  if (map == NULL) {
    return;
  }
  map->error_callback = callback;
  map->error_callback_data = user_data;
}

SamHashMapError samhashmap_get_last_error(const SamHashMap *map) {
  if (map == NULL) {
    return SAMHASHMAP_ERROR_NULL_PARAM;
  }
  return map->last_error;
}

const char *samhashmap_error_string(SamHashMapError error) {
  switch (error) {
    case SAMHASHMAP_ERROR_NONE:
      return "No error";
    case SAMHASHMAP_ERROR_NULL_PARAM:
      return "Null parameter";
    case SAMHASHMAP_ERROR_MEMORY_EXHAUSTED:
      return "Memory exhausted";
    case SAMHASHMAP_ERROR_RESIZE_FAILED:
      return "Resize operation failed";
    case SAMHASHMAP_ERROR_KEY_NOT_FOUND:
      return "Key not found";
    default:
      return "Unknown error";
  }
}

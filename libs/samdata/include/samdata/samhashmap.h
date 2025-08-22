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

#ifndef SAMHASHMAP_H
#define SAMHASHMAP_H

// =============================================================================
// STANDARD INCLUDES
// =============================================================================

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// =============================================================================
// LIBRARY DEPENDENCIES
// =============================================================================

#include <samrena.h>

// =============================================================================
// SAMHASHMAP - Hash Map Data Structure
// =============================================================================

// =============================================================================
// CORE ENUMERATIONS
// =============================================================================

// Hash function type
typedef enum {
  SAMHASHMAP_HASH_DJB2,
  SAMHASHMAP_HASH_FNV1A,
  SAMHASHMAP_HASH_MURMUR3
} SamHashMapHashFunction;

// Error types
typedef enum {
  SAMHASHMAP_ERROR_NONE = 0,
  SAMHASHMAP_ERROR_NULL_PARAM,
  SAMHASHMAP_ERROR_MEMORY_EXHAUSTED,
  SAMHASHMAP_ERROR_RESIZE_FAILED,
  SAMHASHMAP_ERROR_KEY_NOT_FOUND
} SamHashMapError;

// =============================================================================
// CALLBACK FUNCTION TYPES
// =============================================================================

// Error callback function type
typedef void (*SamHashMapErrorCallback)(SamHashMapError error, const char *message,
                                        void *user_data);

// =============================================================================
// PERFORMANCE STRUCTURES
// =============================================================================

// Performance metrics structure
typedef struct {
  size_t total_collisions;
  size_t max_chain_length;
  size_t resize_count;
  double average_chain_length;
  size_t total_operations;
  size_t failed_allocations;
} SamHashMapStats;

// =============================================================================
// CORE STRUCTURES
// =============================================================================

// Define the structure for each key-value pair
typedef struct Cell {
  char *key;
  void *value;
  struct Cell *next;
} Cell;

// Define the main hashmap structure
typedef struct {
  Cell **cells;
  size_t size;                            // Current number of elements
  size_t capacity;                        // Number of buckets
  Samrena *arena;                         // Arena for memory allocation
  float load_factor;                      // Threshold for resizing (default 0.75)
  SamHashMapHashFunction hash_func;       // Hash function to use
  SamHashMapStats stats;                  // Performance metrics
  SamHashMapErrorCallback error_callback; // Error callback function
  void *error_callback_data;              // User data for error callback
  SamHashMapError last_error;             // Last error that occurred
} SamHashMap;

// =============================================================================
// CORE API - SamHashMap Management
// =============================================================================

/**
 * Creates a new samhashmap hash map with default hash function.
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @return New samhashmap instance or NULL if samrena is NULL
 */
SamHashMap *samhashmap_create(size_t initial_capacity, Samrena *samrena);

/**
 * Creates a new samhashmap hash map with specified hash function.
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @param hash_func Hash function to use
 * @return New samhashmap instance or NULL if samrena is NULL
 */
SamHashMap *samhashmap_create_with_hash(size_t initial_capacity, Samrena *samrena,
                                        SamHashMapHashFunction hash_func);

void samhashmap_destroy(SamHashMap *comb);

// =============================================================================
// CORE OPERATIONS API
// =============================================================================

bool samhashmap_put(SamHashMap *comb, const char *key, void *value);
void *samhashmap_get(const SamHashMap *comb, const char *key);

bool samhashmap_remove(SamHashMap *comb, const char *key);
bool samhashmap_contains(const SamHashMap *comb, const char *key);

void samhashmap_clear(SamHashMap *comb);
void samhashmap_print(const SamHashMap *comb);

// =============================================================================
// INFORMATION API
// =============================================================================

size_t samhashmap_size(const SamHashMap *comb);
bool samhashmap_is_empty(const SamHashMap *comb);

// =============================================================================
// COLLECTION API
// =============================================================================

size_t samhashmap_get_keys(const SamHashMap *comb, const char **keys, size_t max_keys);
size_t samhashmap_get_values(const SamHashMap *comb, void **values, size_t max_values);

// =============================================================================
// ITERATOR API
// =============================================================================

typedef void (*SamHashMapIterator)(const char *key, void *value, void *user_data);
void samhashmap_foreach(const SamHashMap *map, SamHashMapIterator iterator, void *user_data);

// =============================================================================
// PERFORMANCE AND DEBUGGING API
// =============================================================================

SamHashMapStats samhashmap_get_stats(const SamHashMap *comb);
void samhashmap_reset_stats(SamHashMap *comb);
void samhashmap_print_stats(const SamHashMap *comb);

// =============================================================================
// ERROR HANDLING API
// =============================================================================

void samhashmap_set_error_callback(SamHashMap *comb, SamHashMapErrorCallback callback,
                                   void *user_data);
SamHashMapError samhashmap_get_last_error(const SamHashMap *comb);
const char *samhashmap_error_string(SamHashMapError error);

// =============================================================================
// TYPE-SAFE WRAPPER MACROS
// =============================================================================
#define SAMHASHMAP_DEFINE_TYPED(name, key_type, value_type)                                        \
  typedef struct name##_samhashmap {                                                               \
    SamHashMap *base;                                                                              \
  } name##_samhashmap;                                                                             \
                                                                                                   \
  static inline name##_samhashmap *name##_create(size_t initial_capacity, Samrena *samrena) {      \
    name##_samhashmap *typed_map = samrena_push(samrena, sizeof(name##_samhashmap));               \
    if (typed_map == NULL)                                                                         \
      return NULL;                                                                                 \
    typed_map->base = samhashmap_create(initial_capacity, samrena);                                \
    if (typed_map->base == NULL)                                                                   \
      return NULL;                                                                                 \
    return typed_map;                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline void name##_destroy(name##_samhashmap *h) {                                        \
    if (h != NULL && h->base != NULL) {                                                            \
      samhashmap_destroy(h->base);                                                                 \
    }                                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_put(name##_samhashmap *h, key_type key, value_type value) {            \
    if (h == NULL || h->base == NULL)                                                              \
      return false;                                                                                \
    return samhashmap_put(h->base, (const char *)key, (void *)value);                              \
  }                                                                                                \
                                                                                                   \
  static inline value_type name##_get(const name##_samhashmap *h, key_type key) {                  \
    if (h == NULL || h->base == NULL)                                                              \
      return (value_type)0;                                                                        \
    return (value_type)samhashmap_get(h->base, (const char *)key);                                 \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_remove(name##_samhashmap *h, key_type key) {                           \
    if (h == NULL || h->base == NULL)                                                              \
      return false;                                                                                \
    return samhashmap_remove(h->base, (const char *)key);                                          \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_contains(const name##_samhashmap *h, key_type key) {                   \
    if (h == NULL || h->base == NULL)                                                              \
      return false;                                                                                \
    return samhashmap_contains(h->base, (const char *)key);                                        \
  }                                                                                                \
                                                                                                   \
  static inline void name##_clear(name##_samhashmap *h) {                                          \
    if (h != NULL && h->base != NULL) {                                                            \
      samhashmap_clear(h->base);                                                                   \
    }                                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline size_t name##_size(const name##_samhashmap *h) {                                   \
    if (h == NULL || h->base == NULL)                                                              \
      return 0;                                                                                    \
    return samhashmap_size(h->base);                                                               \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_is_empty(const name##_samhashmap *h) {                                 \
    if (h == NULL || h->base == NULL)                                                              \
      return true;                                                                                 \
    return samhashmap_is_empty(h->base);                                                           \
  }                                                                                                \
                                                                                                   \
  typedef void (*name##_iterator)(key_type key, value_type value, void *user_data);                \
                                                                                                   \
  static inline void name##_foreach(const name##_samhashmap *h, name##_iterator iterator,          \
                                    void *user_data) {                                             \
    if (h == NULL || h->base == NULL || iterator == NULL)                                          \
      return;                                                                                      \
    samhashmap_foreach(h->base, (SamHashMapIterator)iterator, user_data);                          \
  }

// =============================================================================
// COMMON TYPE-SAFE INSTANTIATIONS
// =============================================================================

SAMHASHMAP_DEFINE_TYPED(string_string, const char *, const char *)
SAMHASHMAP_DEFINE_TYPED(string_int, const char *, int *)
SAMHASHMAP_DEFINE_TYPED(string_ptr, const char *, void *)

#endif // SAMHASHMAP_H
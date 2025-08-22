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

#ifndef SAMSET_H
#define SAMSET_H

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
// SAMSET - Set Data Structure for Unique Elements
// =============================================================================

// =============================================================================
// CORE ENUMERATIONS
// =============================================================================

// Error types
typedef enum {
  SAMSET_ERROR_NONE = 0,
  SAMSET_ERROR_NULL_PARAM,
  SAMSET_ERROR_MEMORY_EXHAUSTED,
  SAMSET_ERROR_RESIZE_FAILED,
  SAMSET_ERROR_ELEMENT_NOT_FOUND,
  SAMSET_ERROR_ELEMENT_EXISTS
} SamSetError;

// Hash function type (reuse from samhashmap)
typedef enum { SAMSET_HASH_DJB2, SAMSET_HASH_FNV1A, SAMSET_HASH_MURMUR3 } SamSetHashFunction;

// =============================================================================
// CALLBACK FUNCTION TYPES
// =============================================================================

// Error callback function type
typedef void (*SamSetErrorCallback)(SamSetError error, const char *message, void *user_data);

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
} SamSetStats;

// =============================================================================
// CORE STRUCTURES
// =============================================================================

// Element node for chaining
typedef struct SamSetNode {
  void *element;
  size_t element_size;
  uint32_t hash;
  struct SamSetNode *next;
} SamSetNode;

// Main set structure
typedef struct {
  SamSetNode **buckets;
  size_t size;                  // Current number of elements
  size_t capacity;              // Number of buckets
  size_t element_size;          // Size of each element
  Samrena *arena;               // Arena for memory allocation
  float load_factor;            // Threshold for resizing (default 0.75)
  SamSetHashFunction hash_func; // Hash function to use

  // Function pointers for custom operations
  uint32_t (*hash)(const void *element, size_t size);
  bool (*equals)(const void *a, const void *b, size_t size);

  // Statistics and error handling
  SamSetStats stats;
  SamSetErrorCallback error_callback;
  void *error_callback_data;
  SamSetError last_error;
} SamSet;

// =============================================================================
// CORE API - SamSet Management
// =============================================================================

/**
 * Creates a new samset set with default hash function.
 * @param element_size Size of each element in bytes
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @return New samset instance or NULL if samrena is NULL
 */
SamSet *samset_create(size_t element_size, size_t initial_capacity, Samrena *samrena);

/**
 * Creates a new samset set with specified hash function.
 * @param element_size Size of each element in bytes
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @param hash_func Hash function to use
 * @return New samset instance or NULL if samrena is NULL
 */
SamSet *samset_create_with_hash(size_t element_size, size_t initial_capacity, Samrena *samrena,
                                SamSetHashFunction hash_func);

/**
 * Creates a new samset set with custom hash and equality functions.
 * @param element_size Size of each element in bytes
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @param hash_fn Custom hash function
 * @param equals_fn Custom equality function
 * @return New samset instance or NULL if samrena is NULL
 */
SamSet *samset_create_custom(size_t element_size, size_t initial_capacity, Samrena *samrena,
                             uint32_t (*hash_fn)(const void *, size_t),
                             bool (*equals_fn)(const void *, const void *, size_t));

void samset_destroy(SamSet *samset);

// =============================================================================
// CORE SET OPERATIONS API
// =============================================================================

bool samset_add(SamSet *samset, const void *element);
bool samset_remove(SamSet *samset, const void *element);
bool samset_contains(const SamSet *samset, const void *element);
void samset_clear(SamSet *samset);

// =============================================================================
// INFORMATION API
// =============================================================================

size_t samset_size(const SamSet *samset);
bool samset_is_empty(const SamSet *samset);

// =============================================================================
// COPY API
// =============================================================================

SamSet *samset_copy(const SamSet *samset, Samrena *samrena);

// =============================================================================
// COLLECTION API
// =============================================================================

size_t samset_to_array(const SamSet *samset, void *array, size_t max_elements);
SamSet *samset_from_array(const void *array, size_t count, size_t element_size, Samrena *samrena);

// =============================================================================
// ITERATOR API
// =============================================================================

typedef void (*SamSetIterator)(const void *element, void *user_data);
void samset_foreach(const SamSet *samset, SamSetIterator iterator, void *user_data);

// =============================================================================
// FUNCTIONAL PROGRAMMING API
// =============================================================================

SamSet *samset_filter(const SamSet *samset, bool (*predicate)(const void *, void *),
                      void *user_data, Samrena *samrena);
SamSet *samset_map(const SamSet *samset, void (*transform)(const void *, void *, void *),
                   size_t new_element_size, void *user_data, Samrena *samrena);

// =============================================================================
// PERFORMANCE AND DEBUGGING API
// =============================================================================

SamSetStats samset_get_stats(const SamSet *samset);
void samset_reset_stats(SamSet *samset);
void samset_print_stats(const SamSet *samset);

// =============================================================================
// ERROR HANDLING API
// =============================================================================

void samset_set_error_callback(SamSet *samset, SamSetErrorCallback callback, void *user_data);
SamSetError samset_get_last_error(const SamSet *samset);
const char *samset_error_string(SamSetError error);

// =============================================================================
// TYPE-SAFE WRAPPER MACROS
// =============================================================================
#define SAMSET_DEFINE_TYPED(name, type)                                                            \
  typedef struct name##_samset {                                                                   \
    SamSet *base;                                                                                  \
  } name##_samset;                                                                                 \
                                                                                                   \
  static inline name##_samset *name##_create(size_t initial_capacity, Samrena *samrena) {          \
    name##_samset *typed_set = samrena_push(samrena, sizeof(name##_samset));                       \
    if (typed_set == NULL)                                                                         \
      return NULL;                                                                                 \
    typed_set->base = samset_create(sizeof(type), initial_capacity, samrena);                      \
    if (typed_set->base == NULL)                                                                   \
      return NULL;                                                                                 \
    return typed_set;                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline void name##_destroy(name##_samset *p) {                                            \
    if (p != NULL && p->base != NULL) {                                                            \
      samset_destroy(p->base);                                                                     \
    }                                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_add(name##_samset *p, type element) {                                  \
    if (p == NULL || p->base == NULL)                                                              \
      return false;                                                                                \
    return samset_add(p->base, &element);                                                          \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_remove(name##_samset *p, type element) {                               \
    if (p == NULL || p->base == NULL)                                                              \
      return false;                                                                                \
    return samset_remove(p->base, &element);                                                       \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_contains(const name##_samset *p, type element) {                       \
    if (p == NULL || p->base == NULL)                                                              \
      return false;                                                                                \
    return samset_contains(p->base, &element);                                                     \
  }                                                                                                \
                                                                                                   \
  static inline void name##_clear(name##_samset *p) {                                              \
    if (p != NULL && p->base != NULL) {                                                            \
      samset_clear(p->base);                                                                       \
    }                                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline size_t name##_size(const name##_samset *p) {                                       \
    if (p == NULL || p->base == NULL)                                                              \
      return 0;                                                                                    \
    return samset_size(p->base);                                                                   \
  }                                                                                                \
                                                                                                   \
  static inline bool name##_is_empty(const name##_samset *p) {                                     \
    if (p == NULL || p->base == NULL)                                                              \
      return true;                                                                                 \
    return samset_is_empty(p->base);                                                               \
  }                                                                                                \
                                                                                                   \
  typedef void (*name##_iterator)(type element, void *user_data);                                  \
                                                                                                   \
  static inline void name##_foreach(const name##_samset *p, name##_iterator iterator,              \
                                    void *user_data) {                                             \
    if (p == NULL || p->base == NULL || iterator == NULL)                                          \
      return;                                                                                      \
    samset_foreach(p->base, (SamSetIterator)iterator, user_data);                                  \
  }

// =============================================================================
// COMMON TYPE-SAFE INSTANTIATIONS
// =============================================================================

SAMSET_DEFINE_TYPED(int_set, int)
SAMSET_DEFINE_TYPED(uint_set, unsigned int)
SAMSET_DEFINE_TYPED(long_set, long)
SAMSET_DEFINE_TYPED(ulong_set, unsigned long)
SAMSET_DEFINE_TYPED(ptr_set, void *)

#endif // SAMSET_H
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

#ifndef SAMVECTOR_H
#define SAMVECTOR_H

#include "samrena.h"
#include <stdbool.h>

// =============================================================================
// VECTOR ERROR CODES
// =============================================================================

typedef enum {
  SAMRENA_VECTOR_SUCCESS = 0,
  SAMRENA_VECTOR_ERROR_NULL_POINTER,
  SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS,
  SAMRENA_VECTOR_ERROR_ALLOCATION_FAILED,
  SAMRENA_VECTOR_ERROR_INVALID_OPERATION,
  SAMRENA_VECTOR_ERROR_ARENA_EXHAUSTED
} SamrenaVectorError;

// =============================================================================
// VECTOR STRUCTURES
// =============================================================================

typedef struct {
  uint64_t size;
  uint64_t element_size;
  uint64_t capacity;
  void *data;
  Samrena *arena;
  bool owns_arena;
  float growth_factor;
  size_t min_growth;
} SamrenaVector;

typedef struct {
  const SamrenaVector *vector;
  size_t current_index;
  bool is_valid;
} SamrenaVectorIterator;

// Vector statistics structure
typedef struct {
  size_t used_bytes;      // size * element_size
  size_t allocated_bytes; // capacity * element_size
  size_t wasted_bytes;    // allocated - used
  float utilization;      // used / allocated ratio
} SamrenaVectorStats;

// =============================================================================
// VECTOR API - Dynamic Arrays
// =============================================================================

SamrenaVector *samrena_vector_init(Samrena *arena, uint64_t element_size,
                                   uint64_t initial_capacity);
SamrenaVector *samrena_vector_init_owned(uint64_t element_size, uint64_t initial_capacity);
void *samrena_vector_push(SamrenaVector *vec, const void *element);
void *samrena_vector_pop(SamrenaVector *vec);
SamrenaVectorError samrena_vector_resize(SamrenaVector *vec, uint64_t new_capacity);

// =============================================================================
// ELEMENT ACCESS API
// =============================================================================

// Safe Access Functions
SamrenaVectorError samrena_vector_get(const SamrenaVector *vec, size_t index, void *out_element);
SamrenaVectorError samrena_vector_set(SamrenaVector *vec, size_t index, const void *element);
void *samrena_vector_at(SamrenaVector *vec, size_t index);
const void *samrena_vector_at_const(const SamrenaVector *vec, size_t index);

// Unsafe Access Functions (Performance)
static inline void *samrena_vector_at_unchecked(SamrenaVector *vec, size_t index);
static inline const void *samrena_vector_at_unchecked_const(const SamrenaVector *vec, size_t index);

// Direct element access macro
#define SAMRENA_VECTOR_ELEM(vec, type, index) ((type *)((vec)->data))[(index)]

// =============================================================================
// TYPE-SAFE VECTOR MACROS
// =============================================================================

// Declare a type-safe vector wrapper for a specific type
#define SAMRENA_DECLARE_VECTOR(type)                                                               \
  typedef struct {                                                                                 \
    SamrenaVector *_vec;                                                                           \
  } SamrenaVector_##type;                                                                          \
                                                                                                   \
  static inline SamrenaVector_##type *samrena_vector_##type##_init(Samrena *arena,                 \
                                                                   uint64_t initial_capacity) {    \
    SamrenaVector_##type *typed_vec =                                                              \
        (SamrenaVector_##type *)samrena_push(arena, sizeof(SamrenaVector_##type));                 \
    if (!typed_vec)                                                                                \
      return NULL;                                                                                 \
    typed_vec->_vec = samrena_vector_init(arena, sizeof(type), initial_capacity);                  \
    if (!typed_vec->_vec)                                                                          \
      return NULL;                                                                                 \
    return typed_vec;                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline SamrenaVector_##type *samrena_vector_##type##_init_owned(                          \
      uint64_t initial_capacity) {                                                                 \
    SamrenaVector_##type *typed_vec =                                                              \
        (SamrenaVector_##type *)malloc(sizeof(SamrenaVector_##type));                              \
    if (!typed_vec)                                                                                \
      return NULL;                                                                                 \
    typed_vec->_vec = samrena_vector_init_owned(sizeof(type), initial_capacity);                   \
    if (!typed_vec->_vec) {                                                                        \
      free(typed_vec);                                                                             \
      return NULL;                                                                                 \
    }                                                                                              \
    return typed_vec;                                                                              \
  }                                                                                                \
                                                                                                   \
  static inline type *samrena_vector_##type##_push(SamrenaVector_##type *vec,                      \
                                                   const type *element) {                          \
    if (!vec || !vec->_vec)                                                                        \
      return NULL;                                                                                 \
    return (type *)samrena_vector_push(vec->_vec, element);                                        \
  }                                                                                                \
                                                                                                   \
  static inline type *samrena_vector_##type##_pop(SamrenaVector_##type *vec) {                     \
    if (!vec || !vec->_vec)                                                                        \
      return NULL;                                                                                 \
    return (type *)samrena_vector_pop(vec->_vec);                                                  \
  }                                                                                                \
                                                                                                   \
  static inline type *samrena_vector_##type##_at(SamrenaVector_##type *vec, size_t index) {        \
    if (!vec || !vec->_vec)                                                                        \
      return NULL;                                                                                 \
    return (type *)samrena_vector_at(vec->_vec, index);                                            \
  }                                                                                                \
                                                                                                   \
  static inline const type *samrena_vector_##type##_at_const(const SamrenaVector_##type *vec,      \
                                                             size_t index) {                       \
    if (!vec || !vec->_vec)                                                                        \
      return NULL;                                                                                 \
    return (const type *)samrena_vector_at_const(vec->_vec, index);                                \
  }                                                                                                \
                                                                                                   \
  static inline SamrenaVectorError samrena_vector_##type##_get(const SamrenaVector_##type *vec,    \
                                                               size_t index, type *out_element) {  \
    if (!vec || !vec->_vec)                                                                        \
      return SAMRENA_VECTOR_ERROR_NULL_POINTER;                                                    \
    return samrena_vector_get(vec->_vec, index, out_element);                                      \
  }                                                                                                \
                                                                                                   \
  static inline SamrenaVectorError samrena_vector_##type##_set(SamrenaVector_##type *vec,          \
                                                               size_t index,                       \
                                                               const type *element) {              \
    if (!vec || !vec->_vec)                                                                        \
      return SAMRENA_VECTOR_ERROR_NULL_POINTER;                                                    \
    return samrena_vector_set(vec->_vec, index, element);                                          \
  }                                                                                                \
                                                                                                   \
  static inline size_t samrena_vector_##type##_size(const SamrenaVector_##type *vec) {             \
    if (!vec || !vec->_vec)                                                                        \
      return 0;                                                                                    \
    return samrena_vector_size(vec->_vec);                                                         \
  }                                                                                                \
                                                                                                   \
  static inline size_t samrena_vector_##type##_capacity(const SamrenaVector_##type *vec) {         \
    if (!vec || !vec->_vec)                                                                        \
      return 0;                                                                                    \
    return samrena_vector_capacity(vec->_vec);                                                     \
  }                                                                                                \
                                                                                                   \
  static inline bool samrena_vector_##type##_is_empty(const SamrenaVector_##type *vec) {           \
    if (!vec || !vec->_vec)                                                                        \
      return true;                                                                                 \
    return samrena_vector_is_empty(vec->_vec);                                                     \
  }                                                                                                \
                                                                                                   \
  static inline bool samrena_vector_##type##_is_full(const SamrenaVector_##type *vec) {            \
    if (!vec || !vec->_vec)                                                                        \
      return true;                                                                                 \
    return samrena_vector_is_full(vec->_vec);                                                      \
  }                                                                                                \
                                                                                                   \
  static inline void samrena_vector_##type##_clear(SamrenaVector_##type *vec) {                    \
    if (vec && vec->_vec)                                                                          \
      samrena_vector_clear(vec->_vec);                                                             \
  }                                                                                                \
                                                                                                   \
  static inline SamrenaVectorError samrena_vector_##type##_resize(SamrenaVector_##type *vec,       \
                                                                  uint64_t new_capacity) {         \
    if (!vec || !vec->_vec)                                                                        \
      return SAMRENA_VECTOR_ERROR_NULL_POINTER;                                                    \
    return samrena_vector_resize(vec->_vec, new_capacity);                                         \
  }                                                                                                \
                                                                                                   \
  static inline void samrena_vector_##type##_destroy(SamrenaVector_##type *vec) {                  \
    if (vec) {                                                                                     \
      if (vec->_vec)                                                                               \
        samrena_vector_destroy(vec->_vec);                                                         \
      if (vec->_vec && vec->_vec->owns_arena)                                                      \
        free(vec);                                                                                 \
    }                                                                                              \
  }

// =============================================================================
// CAPACITY MANAGEMENT API
// =============================================================================

// Content Management Functions
void samrena_vector_clear(SamrenaVector *vec);
SamrenaVectorError samrena_vector_truncate(SamrenaVector *vec, size_t new_size);
SamrenaVectorError samrena_vector_reset(SamrenaVector *vec, size_t initial_capacity);

// Query Functions
size_t samrena_vector_size(const SamrenaVector *vec);
size_t samrena_vector_capacity(const SamrenaVector *vec);
bool samrena_vector_is_empty(const SamrenaVector *vec);
bool samrena_vector_is_full(const SamrenaVector *vec);
size_t samrena_vector_available(const SamrenaVector *vec);

// Vector Lifecycle
// Transfer vector to a new arena by copying all data
// Creates a new vector on new_arena with the same contents as vec
// The original vector remains valid and unchanged
// Returns NULL on allocation failure or invalid input
// Note: Does NOT destroy the original vector - caller manages both lifetimes
SamrenaVector *samrena_vector_transfer(SamrenaVector *vec, Samrena *new_arena);

// Create a new vector containing elements from [start, end)
// Creates a new vector on target_arena with elements from indices [start, end)
// start: inclusive starting index
// end: exclusive ending index (use vec->size to include all remaining elements)
// target_arena: arena for new vector (if NULL, uses vec->arena unless vec owns its arena)
// Returns NULL on allocation failure, invalid input, out of bounds indices,
//   or if target_arena is NULL and vec owns its arena
// Note: Does NOT modify the original vector
SamrenaVector *samrena_vector_slice(const SamrenaVector *vec, size_t start, size_t end,
                                    Samrena *target_arena);

void samrena_vector_destroy(SamrenaVector *vec);

// =============================================================================
// ITERATOR API
// =============================================================================

SamrenaVectorIterator samrena_vector_iter_begin(const SamrenaVector *vec);
bool samrena_vector_iter_has_next(const SamrenaVectorIterator *iter);
const void *samrena_vector_iter_next(SamrenaVectorIterator *iter);
void samrena_vector_iter_reset(SamrenaVectorIterator *iter);

// =============================================================================
// FUNCTIONAL PROGRAMMING API
// =============================================================================

typedef bool (*SamrenaVectorPredicate)(const void *element, void *user_data);
typedef void (*SamrenaVectorTransform)(const void *src, void *dst, void *user_data);
typedef void (*SamrenaVectorForEach)(const void *element, void *user_data);

SamrenaVector *samrena_vector_filter(const SamrenaVector *vec, SamrenaVectorPredicate predicate,
                                     void *user_data, Samrena *target_arena);
SamrenaVector *samrena_vector_map(const SamrenaVector *src_vec, size_t dst_element_size,
                                  SamrenaVectorTransform transform, void *user_data,
                                  Samrena *target_arena);
void samrena_vector_foreach(const SamrenaVector *vec, SamrenaVectorForEach callback,
                            void *user_data);

// =============================================================================
// INLINE IMPLEMENTATIONS
// =============================================================================

#include <assert.h>

static inline void *samrena_vector_at_unchecked(SamrenaVector *vec, size_t index) {
  assert(vec != NULL);
  assert(index < vec->size);
  return (uint8_t *)vec->data + (index * vec->element_size);
}

static inline const void *samrena_vector_at_unchecked_const(const SamrenaVector *vec,
                                                            size_t index) {
  assert(vec != NULL);
  assert(index < vec->size);
  return (const uint8_t *)vec->data + (index * vec->element_size);
}

#endif

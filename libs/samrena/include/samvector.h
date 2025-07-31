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
    const SamrenaVector* vector;
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

SamrenaVector* samrena_vector_init(Samrena* arena, uint64_t element_size, uint64_t initial_capacity);
SamrenaVector* samrena_vector_init_owned(uint64_t element_size, uint64_t initial_capacity);
void* samrena_vector_push(SamrenaVector* vec, const void* element);
void* samrena_vector_pop(SamrenaVector* vec);
SamrenaVectorError samrena_vector_resize(SamrenaVector* vec, uint64_t new_capacity);

// =============================================================================
// ELEMENT ACCESS API
// =============================================================================

// Safe Access Functions
SamrenaVectorError samrena_vector_get(const SamrenaVector* vec, size_t index, void* out_element);
SamrenaVectorError samrena_vector_set(SamrenaVector* vec, size_t index, const void* element);
void* samrena_vector_at(SamrenaVector* vec, size_t index);
const void* samrena_vector_at_const(const SamrenaVector* vec, size_t index);

// Unsafe Access Functions (Performance)
static inline void* samrena_vector_at_unchecked(SamrenaVector* vec, size_t index);
static inline const void* samrena_vector_at_unchecked_const(const SamrenaVector* vec, size_t index);

// Direct element access macro
#define SAMRENA_VECTOR_ELEM(vec, type, index) \
    ((type*)((vec)->data))[(index)]

// =============================================================================
// CAPACITY MANAGEMENT API
// =============================================================================

// Content Management Functions
void samrena_vector_clear(SamrenaVector* vec);
SamrenaVectorError samrena_vector_truncate(SamrenaVector* vec, size_t new_size);
SamrenaVectorError samrena_vector_reset(SamrenaVector* vec, size_t initial_capacity);

// Query Functions
size_t samrena_vector_size(const SamrenaVector* vec);
size_t samrena_vector_capacity(const SamrenaVector* vec);
bool samrena_vector_is_empty(const SamrenaVector* vec);
bool samrena_vector_is_full(const SamrenaVector* vec);
size_t samrena_vector_available(const SamrenaVector* vec);

// Vector Lifecycle
void samrena_vector_destroy(SamrenaVector* vec);

// =============================================================================
// ITERATOR API
// =============================================================================

SamrenaVectorIterator samrena_vector_iter_begin(const SamrenaVector* vec);
bool samrena_vector_iter_has_next(const SamrenaVectorIterator* iter);
const void* samrena_vector_iter_next(SamrenaVectorIterator* iter);
void samrena_vector_iter_reset(SamrenaVectorIterator* iter);

// =============================================================================
// FUNCTIONAL PROGRAMMING API
// =============================================================================

typedef bool (*SamrenaVectorPredicate)(const void* element, void* user_data);
typedef void (*SamrenaVectorTransform)(const void* src, void* dst, void* user_data);
typedef void (*SamrenaVectorForEach)(const void* element, void* user_data);

SamrenaVector* samrena_vector_filter(const SamrenaVector* vec, SamrenaVectorPredicate predicate, void* user_data, Samrena* target_arena);
SamrenaVector* samrena_vector_map(const SamrenaVector* src_vec, size_t dst_element_size, SamrenaVectorTransform transform, void* user_data, Samrena* target_arena);
void samrena_vector_foreach(const SamrenaVector* vec, SamrenaVectorForEach callback, void* user_data);

// =============================================================================
// INLINE IMPLEMENTATIONS
// =============================================================================

#include <assert.h>

static inline void* samrena_vector_at_unchecked(SamrenaVector* vec, size_t index) {
    assert(vec != NULL);
    assert(index < vec->size);
    return (uint8_t*)vec->data + (index * vec->element_size);
}

static inline const void* samrena_vector_at_unchecked_const(const SamrenaVector* vec, size_t index) {
    assert(vec != NULL);
    assert(index < vec->size);
    return (const uint8_t*)vec->data + (index * vec->element_size);
}

#endif

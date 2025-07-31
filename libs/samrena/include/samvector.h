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
  SAMRENA_VECTOR_ERROR_ALLOCATION_FAILED
} SamrenaVectorError;

// =============================================================================
// VECTOR STRUCTURES
// =============================================================================

typedef struct {
  uint64_t size;
  uint64_t element_size;
  uint64_t capacity;
  void *data;
  Samrena *arena;     // Arena used for allocations
  bool owns_arena;    // True if vector owns and should free the arena
  float growth_factor;
  size_t min_growth;
} SamrenaVector;

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
void* samrena_vector_push(Samrena* arena, SamrenaVector* vec, const void* element);
void* samrena_vector_pop(SamrenaVector* vec);
void* samrena_vector_resize(Samrena* arena, SamrenaVector* vec, uint64_t new_capacity);

// =============================================================================
// ELEMENT ACCESS API
// =============================================================================

// Safe Access Functions
SamrenaVectorError samrena_vector_get(const SamrenaVector* vec, size_t index, void* out_element);
SamrenaVectorError samrena_vector_set(SamrenaVector* vec, size_t index, const void* element);
void* samrena_vector_at(SamrenaVector* vec, size_t index);
const void* samrena_vector_at_const(const SamrenaVector* vec, size_t index);

// Convenience Access Functions
void* samrena_vector_front(SamrenaVector* vec);
const void* samrena_vector_front_const(const SamrenaVector* vec);
void* samrena_vector_back(SamrenaVector* vec);
const void* samrena_vector_back_const(const SamrenaVector* vec);
void* samrena_vector_data(SamrenaVector* vec);
const void* samrena_vector_data_const(const SamrenaVector* vec);

// Unsafe Access Functions (Performance)
static inline void* samrena_vector_at_unchecked(SamrenaVector* vec, size_t index);
static inline const void* samrena_vector_at_unchecked_const(const SamrenaVector* vec, size_t index);

// Direct element access macro
#define SAMRENA_VECTOR_ELEM(vec, type, index) \
    ((type*)((vec)->data))[(index)]

// =============================================================================
// CAPACITY MANAGEMENT API
// =============================================================================

// Capacity Control Functions
SamrenaVectorError samrena_vector_reserve(SamrenaVector* vec, size_t min_capacity);
SamrenaVectorError samrena_vector_shrink_to_fit(SamrenaVector* vec);
SamrenaVectorError samrena_vector_set_capacity(SamrenaVector* vec, size_t capacity);

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

// Growth Control Functions
void samrena_vector_set_growth_factor(SamrenaVector* vec, float factor);
void samrena_vector_set_min_growth(SamrenaVector* vec, size_t min_elements);
SamrenaVectorStats samrena_vector_get_stats(const SamrenaVector* vec);

// Memory Ownership Functions
SamrenaVector* samrena_vector_init_owned(uint64_t element_size, uint64_t initial_capacity);
SamrenaVector* samrena_vector_init_with_arena(Samrena* arena, uint64_t element_size, uint64_t initial_capacity);
void samrena_vector_destroy(SamrenaVector* vec);

// Owned Vector Operations
void* samrena_vector_push_owned(SamrenaVector* vec, const void* element);
SamrenaVectorError samrena_vector_reserve_owned(SamrenaVector* vec, size_t min_capacity);

// Auto-detection Functions (Hybrid Operations)
void* samrena_vector_push_auto(SamrenaVector* vec, const void* element);
SamrenaVectorError samrena_vector_reserve_auto(SamrenaVector* vec, size_t min_capacity);
void* samrena_vector_push_with_arena(Samrena* arena, SamrenaVector* vec, const void* element);

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
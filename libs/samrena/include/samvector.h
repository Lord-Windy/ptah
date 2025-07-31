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

// =============================================================================
// VECTOR ERROR CODES
// =============================================================================

typedef enum {
  SAMRENA_VECTOR_SUCCESS = 0,
  SAMRENA_VECTOR_ERROR_NULL_POINTER,
  SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS
} SamrenaVectorError;

// =============================================================================
// VECTOR STRUCTURES
// =============================================================================

typedef struct {
  uint64_t size;
  uint64_t element_size;
  uint64_t capacity;
  void *data;
} SamrenaVector;

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
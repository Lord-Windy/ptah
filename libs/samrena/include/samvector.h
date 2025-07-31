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

#endif
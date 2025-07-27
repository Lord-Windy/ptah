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

#include "samrena.h"
#include <string.h>

#define DEFAULT_CAPACITY 64

SamrenaVector *samrena_vector_init(Samrena *samrena, uint64_t element_size,
                                   uint64_t capacity // 0 uses a default
) {
  // Null pointer check
  if (!samrena) {
    return NULL;
  }

  // Zero element size check
  if (element_size == 0) {
    return NULL;
  }

  uint64_t initial_capacity = capacity ? capacity : DEFAULT_CAPACITY;

  // Check for overflow in data size calculation
  if (initial_capacity > UINT64_MAX / element_size) {
    return NULL;
  }

  SamrenaVector *vec = samrena_push(samrena, sizeof(SamrenaVector));
  if (!vec) {
    return NULL;
  }

  vec->size = 0;
  vec->element_size = element_size;
  vec->capacity = initial_capacity;
  vec->data = samrena_push(samrena, element_size * initial_capacity);

  // Check if data allocation failed
  if (!vec->data) {
    return NULL;
  }

  return vec;
}

void *samrena_vector_push(Samrena *samrena, SamrenaVector *samrena_vector, void *element) {
  // Null pointer checks
  if (!samrena || !samrena_vector || !element) {
    return NULL;
  }

  // Check if we need to resize the vector
  if (samrena_vector->size >= samrena_vector->capacity) {
    void *resize_result = samrena_vector_resize(samrena, samrena_vector, 2); // Double the size
    if (!resize_result) {
      return NULL; // Resize failed
    }
  }

  // Check for overflow in offset calculation
  if (samrena_vector->size > UINT64_MAX / samrena_vector->element_size) {
    return NULL;
  }

  // Calculate the position to place the new element
  uint8_t *target =
      (uint8_t *)samrena_vector->data + (samrena_vector->size * samrena_vector->element_size);

  // Copy the element data to the target position using memcpy
  memcpy(target, element, samrena_vector->element_size);

  // Increment the size
  samrena_vector->size++;

  // Return the pointer to the newly added element
  return (void *)target;
}

void *samrena_vector_pop(SamrenaVector *samrena_vector) {
  // Null pointer check
  if (!samrena_vector) {
    return NULL;
  }

  // Check if the vector is empty
  if (samrena_vector->size == 0) {
    return NULL;
  }

  // Decrement the size
  samrena_vector->size--;

  // Calculate and return the pointer to the last element
  return (uint8_t *)samrena_vector->data + (samrena_vector->size * samrena_vector->element_size);
}

void *samrena_vector_resize(Samrena *samrena, SamrenaVector *samrena_vector,
                            uint64_t resize_factor) {
  // Null pointer checks
  if (!samrena || !samrena_vector) {
    return NULL;
  }

  // Zero resize factor check
  if (resize_factor == 0) {
    return NULL;
  }

  // Check for overflow in capacity calculation
  if (samrena_vector->capacity > UINT64_MAX / resize_factor) {
    return NULL;
  }

  // Calculate the new capacity
  uint64_t new_capacity = samrena_vector->capacity * resize_factor;

  // Check for overflow in data size calculation
  if (new_capacity > UINT64_MAX / samrena_vector->element_size) {
    return NULL;
  }

  // Allocate a new data block with the new capacity
  void *new_data = samrena_push(samrena, new_capacity * samrena_vector->element_size);

  // Check if allocation was successful
  if (!new_data) {
    return NULL;
  }

  // Copy existing data to the new memory using memcpy
  if (samrena_vector->size > 0 && samrena_vector->data) {
    // Check for overflow in bytes calculation
    if (samrena_vector->size > UINT64_MAX / samrena_vector->element_size) {
      return NULL;
    }
    uint64_t bytes_to_copy = samrena_vector->size * samrena_vector->element_size;
    memcpy(new_data, samrena_vector->data, bytes_to_copy);
  }

  // Update the vector with the new data and capacity
  samrena_vector->data = new_data;
  samrena_vector->capacity = new_capacity;

  return new_data;
}

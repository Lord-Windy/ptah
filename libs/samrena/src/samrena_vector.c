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

  uint64_t initial_capacity = capacity ? capacity : DEFAULT_CAPACITY;
  SamrenaVector *vec = samrena_push(samrena, sizeof(SamrenaVector));

  vec->size = 0;
  vec->element_size = element_size;
  vec->capacity = initial_capacity;
  vec->data = samrena_push(samrena, element_size * initial_capacity);

  return vec;
}

void *samrena_vector_push(SamrenaVector *samrena_vector, Samrena *samrena, void *element) {
  // Check if we need to resize the vector
  if (samrena_vector->size >= samrena_vector->capacity) {
    samrena_vector_resize(samrena_vector, samrena, 2); // Double the size
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
  // Check if the vector is empty
  if (samrena_vector->size == 0) {
    return NULL;
  }

  // Decrement the size
  samrena_vector->size--;

  // Calculate and return the pointer to the last element
  return (uint8_t *)samrena_vector->data + (samrena_vector->size * samrena_vector->element_size);
}

void *samrena_vector_resize(SamrenaVector *samrena_vector, Samrena *samrena,
                            uint64_t resize_factor) {
  // Calculate the new capacity
  uint64_t new_capacity = samrena_vector->capacity * resize_factor;

  // Allocate a new data block with the new capacity
  void *new_data = samrena_push(samrena, new_capacity * samrena_vector->element_size);

  // Check if allocation was successful
  if (!new_data) {
    return NULL;
  }

  // Copy existing data to the new memory using memcpy
  uint64_t bytes_to_copy = samrena_vector->size * samrena_vector->element_size;
  memcpy(new_data, samrena_vector->data, bytes_to_copy);

  // Update the vector with the new data and capacity
  samrena_vector->data = new_data;
  samrena_vector->capacity = new_capacity;

  return new_data;
}

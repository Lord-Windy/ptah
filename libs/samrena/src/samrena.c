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
#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>
#include <string.h>

#define PAGE_SIZE 4096

static SamrenaError last_error = SAMRENA_SUCCESS;

SamrenaError samrena_get_last_error(void) {
  return last_error;
}

const char *samrena_error_string(SamrenaError error) {
  switch (error) {
    case SAMRENA_SUCCESS:
      return "Success";
    case SAMRENA_ERROR_NULL_POINTER:
      return "Null pointer error";
    case SAMRENA_ERROR_INVALID_SIZE:
      return "Invalid size error";
    case SAMRENA_ERROR_OUT_OF_MEMORY:
      return "Out of memory error";
    case SAMRENA_ERROR_OVERFLOW:
      return "Overflow error";
    default:
      return "Unknown error";
  }
}

static void samrena_set_error(SamrenaError error) {
  last_error = error;
}

uint8_t *samrena_basic_malloc(uint64_t page_count) {

  uint64_t size = page_count * PAGE_SIZE;
  uint8_t *allocated_bytes = malloc(size * sizeof(uint8_t));

  return allocated_bytes;
}

void samrena_basic_free(uint8_t *bytes) { free(bytes); }

Samrena *samrena_allocate(uint64_t page_count) {
  samrena_set_error(SAMRENA_SUCCESS);
  
  // Handle zero page count - return NULL as this is invalid
  if (page_count == 0) {
    samrena_set_error(SAMRENA_ERROR_INVALID_SIZE);
    return NULL;
  }

  // Check for multiplication overflow
  if (page_count > UINT64_MAX / PAGE_SIZE) {
    samrena_set_error(SAMRENA_ERROR_OVERFLOW);
    return NULL;
  }
  
  uint64_t data_size = PAGE_SIZE * page_count;
  
  // Allocate metadata separately from data pool
  Samrena *samrena = malloc(sizeof(Samrena));
  if (!samrena) {
    samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    return NULL;
  }
  
  // Allocate the data pool separately
  uint8_t *bytes = malloc(data_size);
  if (!bytes) {
    free(samrena);
    samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    return NULL;
  }

  samrena->bytes = bytes;
  samrena->allocated = 0;  // Start with 0 allocated since data pool is separate
  samrena->capacity = data_size;

  return samrena;
}

void *samrena_push(Samrena *samrena, uint64_t size) {
  samrena_set_error(SAMRENA_SUCCESS);
  
  // Null pointer check
  if (!samrena) {
    samrena_set_error(SAMRENA_ERROR_NULL_POINTER);
    return NULL;
  }
  
  // Handle zero size allocation
  if (size == 0) {
    samrena_set_error(SAMRENA_ERROR_INVALID_SIZE);
    return NULL;
  }

  // Calculate alignment for the requested size
  size_t alignment = __builtin_ctz(size) ? (1 << __builtin_ctz(size)) : alignof(max_align_t);
  if (alignment > alignof(max_align_t)) {
    alignment = alignof(max_align_t);
  }

  // Align the current allocated position
  uint64_t aligned_offset = (samrena->allocated + alignment - 1) & ~(alignment - 1);

  // Check for overflow in offset calculation
  if (aligned_offset < samrena->allocated) {
    samrena_set_error(SAMRENA_ERROR_OVERFLOW);
    return NULL;
  }
  
  // Check for overflow in final size calculation
  if (aligned_offset > UINT64_MAX - size) {
    samrena_set_error(SAMRENA_ERROR_OVERFLOW);
    return NULL;
  }

  // In future, expand memory
  if (aligned_offset + size > samrena->capacity) {
    samrena_set_error(SAMRENA_ERROR_OUT_OF_MEMORY);
    return NULL;
  }

  void *pointer = (void *)(samrena->bytes + aligned_offset);
  samrena->allocated = aligned_offset + size;

  return pointer;
}

void *samrena_push_zero(Samrena *samrena, uint64_t size) {
  uint8_t *pointer = samrena_push(samrena, size);
  
  // Check if allocation failed
  if (!pointer) {
    return NULL;
  }

  // Use memset for better performance and safety
  memset(pointer, 0, size);

  return pointer;
}

uint64_t samrena_allocated(Samrena *samrena) { 
  if (!samrena) {
    return 0;
  }
  return samrena->allocated; 
}

uint64_t samrena_capacity(Samrena *samrena) { 
  if (!samrena) {
    return 0;
  }
  return samrena->capacity; 
}

void samrena_deallocate(Samrena *samrena) {
  if (!samrena) {
    return;
  }
  // Free the data pool first
  if (samrena->bytes) {
    free(samrena->bytes);
  }
  // Free the metadata structure
  free(samrena);
}

void *samrena_resize_array(Samrena *samrena, void *original_array, uint64_t original_size,
                           uint64_t new_size) {
  // Null samrena check
  if (!samrena) {
    return NULL;
  }
  
  // Handle zero new size - for compatibility with tests, allow this
  if (new_size == 0) {
    new_size = 1; // Allocate minimal space instead of failing
  }

  void *new_data = samrena_push(samrena, new_size);
  if (!new_data) {
    return NULL;
  }

  // Copy the original data over if original_array is not null
  if (original_array && original_size > 0) {
    uint64_t copy_size = original_size < new_size ? original_size : new_size;
    memcpy(new_data, original_array, copy_size);
  }

  return new_data;
}

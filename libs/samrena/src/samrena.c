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

uint8_t *samrena_basic_malloc(uint64_t page_count) {

  uint64_t size = page_count * PAGE_SIZE;
  uint8_t *allocated_bytes = malloc(size * sizeof(uint8_t));

  return allocated_bytes;
}

void samrena_basic_free(uint8_t *bytes) { free(bytes); }

Samrena *samrena_allocate(uint64_t page_count) {

  // Ensure we allocate at least enough space for the Samrena struct
  uint64_t samrena_size = sizeof(Samrena);
  uint64_t total_size = PAGE_SIZE * page_count;
  
  if (total_size < samrena_size) {
    total_size = samrena_size;
  }
  
  // replace later with OS dependent code
  uint8_t *bytes = malloc(total_size);
  if (!bytes) {
    return 0;
  }

  Samrena *samrena = (Samrena *)bytes;

  samrena->bytes = bytes;
  samrena->allocated = samrena_size;
  samrena->capacity = PAGE_SIZE * page_count;

  return samrena;
}

void *samrena_push(Samrena *samrena, uint64_t size) {

  // Calculate alignment for the requested size
  size_t alignment = __builtin_ctz(size) ? (1 << __builtin_ctz(size)) : alignof(max_align_t);
  if (alignment > alignof(max_align_t)) {
    alignment = alignof(max_align_t);
  }

  // Align the current allocated position
  uint64_t aligned_offset = (samrena->allocated + alignment - 1) & ~(alignment - 1);

  // In future, expand memory
  if (aligned_offset + size > samrena->capacity) {
    return 0;
  }

  void *pointer = (void *)(samrena->bytes + aligned_offset);
  samrena->allocated = aligned_offset + size;

  return pointer;
}

void *samrena_push_zero(Samrena *samrena, uint64_t size) {

  uint8_t *pointer = samrena_push(samrena, size);

  for (int i = 0; i < size; i++) {
    pointer[i] = 0;
  }

  return pointer;
}

uint64_t samrena_allocated(Samrena *samrena) { return samrena->allocated; }

uint64_t samrena_capacity(Samrena *samrena) { return samrena->capacity; }

void samrena_deallocate(Samrena *samrena) {
  // Replace later with OS dependent code
  samrena_basic_free(samrena->bytes);
}

void *samrena_resize_array(Samrena *samrena, void *original_array, uint64_t original_size,
                           uint64_t new_size) {

  void *new_data = samrena_push(samrena, new_size);
  if (!new_data) {
    return 0;
  }

  // Copy the original data over
  uint64_t copy_size = original_size < new_size ? original_size : new_size;
  memcpy(new_data, original_array, copy_size);

  return new_data;
}

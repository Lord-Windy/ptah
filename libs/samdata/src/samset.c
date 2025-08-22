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

#include <samdata/samhash.h>
#include <samdata/samset.h>
#include <stdio.h>
#include <string.h>

// =============================================================================
// INTERNAL CONSTANTS
// =============================================================================

#define SAMSET_DEFAULT_LOAD_FACTOR 0.75f
#define SAMSET_MIN_CAPACITY 16

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static uint32_t (*samset_get_hash_function(SamSetHashFunction func))(const void *, size_t) {
  switch (func) {
    case SAMSET_HASH_DJB2:
      return samhash_djb2;
    case SAMSET_HASH_FNV1A:
      return samhash_fnv1a;
    case SAMSET_HASH_MURMUR3:
      return samhash_murmur3;
    default:
      return samhash_djb2;
  }
}

static bool samset_default_equals(const void *a, const void *b, size_t size) {
  return memcmp(a, b, size) == 0;
}

static void samset_set_error(SamSet *samset, SamSetError error) {
  if (samset == NULL)
    return;

  samset->last_error = error;

  if (samset->error_callback) {
    samset->error_callback(error, samset_error_string(error), samset->error_callback_data);
  }
}

// =============================================================================
// CORE SAMSET MANAGEMENT
// =============================================================================

SamSet *samset_create(size_t element_size, size_t initial_capacity, Samrena *samrena) {
  return samset_create_with_hash(element_size, initial_capacity, samrena, SAMSET_HASH_DJB2);
}

SamSet *samset_create_with_hash(size_t element_size, size_t initial_capacity, Samrena *samrena,
                                SamSetHashFunction hash_func) {
  return samset_create_custom(element_size, initial_capacity, samrena,
                              samset_get_hash_function(hash_func), samset_default_equals);
}

SamSet *samset_create_custom(size_t element_size, size_t initial_capacity, Samrena *samrena,
                             uint32_t (*hash_fn)(const void *, size_t),
                             bool (*equals_fn)(const void *, const void *, size_t)) {
  if (samrena == NULL) {
    return NULL;
  }

  if (element_size == 0) {
    return NULL;
  }

  if (initial_capacity < SAMSET_MIN_CAPACITY) {
    initial_capacity = SAMSET_MIN_CAPACITY;
  }

  SamSet *samset = samrena_push(samrena, sizeof(SamSet));
  if (samset == NULL) {
    return NULL;
  }

  samset->buckets = samrena_push(samrena, sizeof(SamSetNode *) * initial_capacity);
  if (samset->buckets == NULL) {
    return NULL;
  }

  memset(samset->buckets, 0, sizeof(SamSetNode *) * initial_capacity);

  samset->size = 0;
  samset->capacity = initial_capacity;
  samset->element_size = element_size;
  samset->arena = samrena;
  samset->load_factor = SAMSET_DEFAULT_LOAD_FACTOR;
  samset->hash_func = SAMSET_HASH_DJB2;

  samset->hash = hash_fn ? hash_fn : samhash_djb2;
  samset->equals = equals_fn ? equals_fn : samset_default_equals;

  memset(&samset->stats, 0, sizeof(SamSetStats));
  samset->error_callback = NULL;
  samset->error_callback_data = NULL;
  samset->last_error = SAMSET_ERROR_NONE;

  return samset;
}

void samset_destroy(SamSet *samset) {
  if (samset == NULL) {
    return;
  }

  samset->buckets = NULL;
  samset->size = 0;
  samset->capacity = 0;
  samset->arena = NULL;
  samset->hash = NULL;
  samset->equals = NULL;
  samset->error_callback = NULL;
  samset->error_callback_data = NULL;
}

// =============================================================================
// ERROR HANDLING
// =============================================================================

const char *samset_error_string(SamSetError error) {
  switch (error) {
    case SAMSET_ERROR_NONE:
      return "No error";
    case SAMSET_ERROR_NULL_PARAM:
      return "Null parameter provided";
    case SAMSET_ERROR_MEMORY_EXHAUSTED:
      return "Memory exhausted";
    case SAMSET_ERROR_RESIZE_FAILED:
      return "Failed to resize set";
    case SAMSET_ERROR_ELEMENT_NOT_FOUND:
      return "Element not found";
    case SAMSET_ERROR_ELEMENT_EXISTS:
      return "Element already exists";
    default:
      return "Unknown error";
  }
}

void samset_set_error_callback(SamSet *samset, SamSetErrorCallback callback, void *user_data) {
  if (samset == NULL)
    return;

  samset->error_callback = callback;
  samset->error_callback_data = user_data;
}

SamSetError samset_get_last_error(const SamSet *samset) {
  if (samset == NULL)
    return SAMSET_ERROR_NULL_PARAM;
  return samset->last_error;
}

// =============================================================================
// RESIZE HELPER FUNCTION
// =============================================================================

static bool samset_resize(SamSet *samset, size_t new_capacity) {
  if (samset == NULL)
    return false;

  SamSetNode **old_buckets = samset->buckets;
  size_t old_capacity = samset->capacity;

  SamSetNode **new_buckets = samrena_push(samset->arena, sizeof(SamSetNode *) * new_capacity);
  if (new_buckets == NULL) {
    samset_set_error(samset, SAMSET_ERROR_MEMORY_EXHAUSTED);
    samset->stats.failed_allocations++;
    return false;
  }

  memset(new_buckets, 0, sizeof(SamSetNode *) * new_capacity);

  samset->buckets = new_buckets;
  samset->capacity = new_capacity;
  samset->stats.resize_count++;

  for (size_t i = 0; i < old_capacity; i++) {
    SamSetNode *current = old_buckets[i];
    while (current != NULL) {
      SamSetNode *next = current->next;

      size_t bucket_index = current->hash % new_capacity;
      current->next = new_buckets[bucket_index];
      new_buckets[bucket_index] = current;

      current = next;
    }
  }

  return true;
}

// =============================================================================
// CORE SET OPERATIONS
// =============================================================================

bool samset_add(SamSet *samset, const void *element) {
  if (samset == NULL || element == NULL) {
    if (samset)
      samset_set_error(samset, SAMSET_ERROR_NULL_PARAM);
    return false;
  }

  samset->stats.total_operations++;

  uint32_t hash = samset->hash(element, samset->element_size);
  size_t bucket_index = hash % samset->capacity;

  SamSetNode *current = samset->buckets[bucket_index];
  size_t chain_length = 0;

  while (current != NULL) {
    chain_length++;
    if (current->hash == hash && samset->equals(current->element, element, samset->element_size)) {
      samset_set_error(samset, SAMSET_ERROR_ELEMENT_EXISTS);
      return false;
    }
    current = current->next;
  }

  if (chain_length > 0) {
    samset->stats.total_collisions++;
    if (chain_length > samset->stats.max_chain_length) {
      samset->stats.max_chain_length = chain_length;
    }
  }

  if ((float)(samset->size + 1) / samset->capacity > samset->load_factor) {
    if (!samset_resize(samset, samset->capacity * 2)) {
      samset_set_error(samset, SAMSET_ERROR_RESIZE_FAILED);
      return false;
    }
    bucket_index = hash % samset->capacity;
  }

  SamSetNode *new_node = samrena_push(samset->arena, sizeof(SamSetNode));
  if (new_node == NULL) {
    samset_set_error(samset, SAMSET_ERROR_MEMORY_EXHAUSTED);
    samset->stats.failed_allocations++;
    return false;
  }

  new_node->element = samrena_push(samset->arena, samset->element_size);
  if (new_node->element == NULL) {
    samset_set_error(samset, SAMSET_ERROR_MEMORY_EXHAUSTED);
    samset->stats.failed_allocations++;
    return false;
  }

  memcpy(new_node->element, element, samset->element_size);
  new_node->hash = hash;
  new_node->element_size = samset->element_size;
  new_node->next = samset->buckets[bucket_index];
  samset->buckets[bucket_index] = new_node;

  samset->size++;
  samset_set_error(samset, SAMSET_ERROR_NONE);
  return true;
}

bool samset_contains(const SamSet *samset, const void *element) {
  if (samset == NULL || element == NULL) {
    return false;
  }

  uint32_t hash = samset->hash(element, samset->element_size);
  size_t bucket_index = hash % samset->capacity;

  SamSetNode *current = samset->buckets[bucket_index];
  while (current != NULL) {
    if (current->hash == hash && samset->equals(current->element, element, samset->element_size)) {
      return true;
    }
    current = current->next;
  }

  return false;
}

bool samset_remove(SamSet *samset, const void *element) {
  if (samset == NULL || element == NULL) {
    if (samset)
      samset_set_error(samset, SAMSET_ERROR_NULL_PARAM);
    return false;
  }

  samset->stats.total_operations++;

  uint32_t hash = samset->hash(element, samset->element_size);
  size_t bucket_index = hash % samset->capacity;

  SamSetNode **current_ptr = &samset->buckets[bucket_index];

  while (*current_ptr != NULL) {
    SamSetNode *current = *current_ptr;
    if (current->hash == hash && samset->equals(current->element, element, samset->element_size)) {

      *current_ptr = current->next;
      samset->size--;
      samset_set_error(samset, SAMSET_ERROR_NONE);
      return true;
    }
    current_ptr = &current->next;
  }

  samset_set_error(samset, SAMSET_ERROR_ELEMENT_NOT_FOUND);
  return false;
}

void samset_clear(SamSet *samset) {
  if (samset == NULL)
    return;

  for (size_t i = 0; i < samset->capacity; i++) {
    samset->buckets[i] = NULL;
  }

  samset->size = 0;
  samset_set_error(samset, SAMSET_ERROR_NONE);
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

size_t samset_size(const SamSet *samset) {
  if (samset == NULL)
    return 0;
  return samset->size;
}

bool samset_is_empty(const SamSet *samset) {
  if (samset == NULL)
    return true;
  return samset->size == 0;
}

// =============================================================================
// PERFORMANCE AND DEBUGGING
// =============================================================================

SamSetStats samset_get_stats(const SamSet *samset) {
  SamSetStats empty_stats = {0};
  if (samset == NULL)
    return empty_stats;

  SamSetStats stats = samset->stats;

  if (samset->capacity > 0 && samset->size > 0) {
    size_t total_chain_length = 0;
    size_t non_empty_buckets = 0;

    for (size_t i = 0; i < samset->capacity; i++) {
      size_t chain_length = 0;
      SamSetNode *current = samset->buckets[i];

      while (current != NULL) {
        chain_length++;
        current = current->next;
      }

      if (chain_length > 0) {
        non_empty_buckets++;
        total_chain_length += chain_length;
      }
    }

    if (non_empty_buckets > 0) {
      stats.average_chain_length = (double)total_chain_length / non_empty_buckets;
    }
  }

  return stats;
}

void samset_reset_stats(SamSet *samset) {
  if (samset == NULL)
    return;
  memset(&samset->stats, 0, sizeof(SamSetStats));
}

void samset_print_stats(const SamSet *samset) {
  if (samset == NULL)
    return;

  SamSetStats stats = samset_get_stats(samset);

  printf("SamSet Statistics:\n");
  printf("  Size: %zu elements\n", samset->size);
  printf("  Capacity: %zu buckets\n", samset->capacity);
  printf("  Load Factor: %.2f\n",
         samset->capacity > 0 ? (double)samset->size / samset->capacity : 0.0);
  printf("  Total Operations: %zu\n", stats.total_operations);
  printf("  Total Collisions: %zu\n", stats.total_collisions);
  printf("  Max Chain Length: %zu\n", stats.max_chain_length);
  printf("  Average Chain Length: %.2f\n", stats.average_chain_length);
  printf("  Resize Count: %zu\n", stats.resize_count);
  printf("  Failed Allocations: %zu\n", stats.failed_allocations);
}

// =============================================================================
// ITERATOR FUNCTIONS
// =============================================================================

void samset_foreach(const SamSet *samset, SamSetIterator iterator, void *user_data) {
  if (samset == NULL || iterator == NULL)
    return;

  for (size_t i = 0; i < samset->capacity; i++) {
    SamSetNode *current = samset->buckets[i];
    while (current != NULL) {
      iterator(current->element, user_data);
      current = current->next;
    }
  }
}

// =============================================================================
// COPY FUNCTIONS
// =============================================================================

SamSet *samset_copy(const SamSet *samset, Samrena *samrena) {
  if (samset == NULL || samrena == NULL) {
    return NULL;
  }

  SamSet *copy = samset_create_custom(samset->element_size, samset->capacity, samrena, samset->hash,
                                      samset->equals);
  if (copy == NULL) {
    return NULL;
  }

  copy->load_factor = samset->load_factor;
  copy->hash_func = samset->hash_func;

  for (size_t i = 0; i < samset->capacity; i++) {
    SamSetNode *current = samset->buckets[i];
    while (current != NULL) {
      if (!samset_add(copy, current->element)) {
        return NULL;
      }
      current = current->next;
    }
  }

  return copy;
}

// =============================================================================
// COLLECTION CONVERSION FUNCTIONS
// =============================================================================

size_t samset_to_array(const SamSet *samset, void *array, size_t max_elements) {
  if (samset == NULL || array == NULL || max_elements == 0) {
    return 0;
  }

  char *arr = (char *)array;
  size_t count = 0;

  for (size_t i = 0; i < samset->capacity && count < max_elements; i++) {
    SamSetNode *current = samset->buckets[i];
    while (current != NULL && count < max_elements) {
      memcpy(arr + (count * samset->element_size), current->element, samset->element_size);
      count++;
      current = current->next;
    }
  }

  return count;
}

SamSet *samset_from_array(const void *array, size_t count, size_t element_size, Samrena *samrena) {
  if (array == NULL || count == 0 || element_size == 0 || samrena == NULL) {
    return NULL;
  }

  size_t initial_capacity = count > SAMSET_MIN_CAPACITY ? count * 2 : SAMSET_MIN_CAPACITY;
  SamSet *samset = samset_create(element_size, initial_capacity, samrena);
  if (samset == NULL) {
    return NULL;
  }

  const char *arr = (const char *)array;
  for (size_t i = 0; i < count; i++) {
    const void *element = arr + (i * element_size);
    samset_add(samset, element);
  }

  return samset;
}

// =============================================================================
// FUNCTIONAL PROGRAMMING FUNCTIONS
// =============================================================================

SamSet *samset_filter(const SamSet *samset, bool (*predicate)(const void *, void *),
                      void *user_data, Samrena *samrena) {
  if (samset == NULL || predicate == NULL || samrena == NULL) {
    return NULL;
  }

  size_t initial_capacity = samset->size > 0 ? samset->size : SAMSET_MIN_CAPACITY;
  SamSet *filtered = samset_create_custom(samset->element_size, initial_capacity, samrena,
                                          samset->hash, samset->equals);
  if (filtered == NULL) {
    return NULL;
  }

  filtered->load_factor = samset->load_factor;
  filtered->hash_func = samset->hash_func;

  for (size_t i = 0; i < samset->capacity; i++) {
    SamSetNode *current = samset->buckets[i];
    while (current != NULL) {
      if (predicate(current->element, user_data)) {
        if (!samset_add(filtered, current->element)) {
          return NULL;
        }
      }
      current = current->next;
    }
  }

  return filtered;
}

SamSet *samset_map(const SamSet *samset, void (*transform)(const void *, void *, void *),
                   size_t new_element_size, void *user_data, Samrena *samrena) {
  if (samset == NULL || transform == NULL || samrena == NULL || new_element_size == 0) {
    return NULL;
  }

  size_t initial_capacity = samset->size > 0 ? samset->size * 2 : SAMSET_MIN_CAPACITY;
  SamSet *mapped = samset_create(new_element_size, initial_capacity, samrena);
  if (mapped == NULL) {
    return NULL;
  }

  mapped->load_factor = samset->load_factor;

  void *temp_element = samrena_push(samrena, new_element_size);
  if (temp_element == NULL) {
    return NULL;
  }

  for (size_t i = 0; i < samset->capacity; i++) {
    SamSetNode *current = samset->buckets[i];
    while (current != NULL) {
      transform(current->element, temp_element, user_data);

      if (!samset_add(mapped, temp_element)) {
        samset_set_error(mapped, SAMSET_ERROR_MEMORY_EXHAUSTED);
        return NULL;
      }
      current = current->next;
    }
  }

  return mapped;
}
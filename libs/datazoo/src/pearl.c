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

#include <datazoo/pearl.h>
#include <datazoo/starfish.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// INTERNAL CONSTANTS
// =============================================================================

#define PEARL_DEFAULT_LOAD_FACTOR 0.75f
#define PEARL_MIN_CAPACITY 16


// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static uint32_t (*pearl_get_hash_function(PearlHashFunction func))(const void *, size_t) {
    switch (func) {
        case PEARL_HASH_DJB2:
            return starfish_hash_djb2;
        case PEARL_HASH_FNV1A:
            return starfish_hash_fnv1a;
        case PEARL_HASH_MURMUR3:
            return starfish_hash_murmur3;
        default:
            return starfish_hash_djb2;
    }
}

static bool pearl_default_equals(const void *a, const void *b, size_t size) {
    return memcmp(a, b, size) == 0;
}

static void pearl_set_error(Pearl *pearl, PearlError error) {
    if (pearl == NULL) return;
    
    pearl->last_error = error;
    
    if (pearl->error_callback) {
        pearl->error_callback(error, pearl_error_string(error), pearl->error_callback_data);
    }
}

// =============================================================================
// CORE PEARL MANAGEMENT
// =============================================================================

Pearl *pearl_create(size_t element_size, size_t initial_capacity, Samrena *samrena) {
    return pearl_create_with_hash(element_size, initial_capacity, samrena, PEARL_HASH_DJB2);
}

Pearl *pearl_create_with_hash(size_t element_size, size_t initial_capacity, 
                              Samrena *samrena, PearlHashFunction hash_func) {
    return pearl_create_custom(element_size, initial_capacity, samrena,
                              pearl_get_hash_function(hash_func),
                              pearl_default_equals);
}

Pearl *pearl_create_custom(size_t element_size, size_t initial_capacity, Samrena *samrena,
                          uint32_t (*hash_fn)(const void *, size_t),
                          bool (*equals_fn)(const void *, const void *, size_t)) {
    if (samrena == NULL) {
        return NULL;
    }
    
    if (element_size == 0) {
        return NULL;
    }
    
    if (initial_capacity < PEARL_MIN_CAPACITY) {
        initial_capacity = PEARL_MIN_CAPACITY;
    }
    
    Pearl *pearl = samrena_push(samrena, sizeof(Pearl));
    if (pearl == NULL) {
        return NULL;
    }
    
    pearl->buckets = samrena_push(samrena, sizeof(PearlNode *) * initial_capacity);
    if (pearl->buckets == NULL) {
        return NULL;
    }
    
    memset(pearl->buckets, 0, sizeof(PearlNode *) * initial_capacity);
    
    pearl->size = 0;
    pearl->capacity = initial_capacity;
    pearl->element_size = element_size;
    pearl->arena = samrena;
    pearl->load_factor = PEARL_DEFAULT_LOAD_FACTOR;
    pearl->hash_func = PEARL_HASH_DJB2;
    
    pearl->hash = hash_fn ? hash_fn : starfish_hash_djb2;
    pearl->equals = equals_fn ? equals_fn : pearl_default_equals;
    
    memset(&pearl->stats, 0, sizeof(PearlStats));
    pearl->error_callback = NULL;
    pearl->error_callback_data = NULL;
    pearl->last_error = PEARL_ERROR_NONE;
    
    return pearl;
}

void pearl_destroy(Pearl *pearl) {
    if (pearl == NULL) {
        return;
    }
    
    pearl->buckets = NULL;
    pearl->size = 0;
    pearl->capacity = 0;
    pearl->arena = NULL;
    pearl->hash = NULL;
    pearl->equals = NULL;
    pearl->error_callback = NULL;
    pearl->error_callback_data = NULL;
}

// =============================================================================
// ERROR HANDLING
// =============================================================================

const char *pearl_error_string(PearlError error) {
    switch (error) {
        case PEARL_ERROR_NONE:
            return "No error";
        case PEARL_ERROR_NULL_PARAM:
            return "Null parameter provided";
        case PEARL_ERROR_MEMORY_EXHAUSTED:
            return "Memory exhausted";
        case PEARL_ERROR_RESIZE_FAILED:
            return "Failed to resize set";
        case PEARL_ERROR_ELEMENT_NOT_FOUND:
            return "Element not found";
        case PEARL_ERROR_ELEMENT_EXISTS:
            return "Element already exists";
        default:
            return "Unknown error";
    }
}

void pearl_set_error_callback(Pearl *pearl, PearlErrorCallback callback, void *user_data) {
    if (pearl == NULL) return;
    
    pearl->error_callback = callback;
    pearl->error_callback_data = user_data;
}

PearlError pearl_get_last_error(const Pearl *pearl) {
    if (pearl == NULL) return PEARL_ERROR_NULL_PARAM;
    return pearl->last_error;
}

// =============================================================================
// RESIZE HELPER FUNCTION
// =============================================================================

static bool pearl_resize(Pearl *pearl, size_t new_capacity) {
    if (pearl == NULL) return false;
    
    PearlNode **old_buckets = pearl->buckets;
    size_t old_capacity = pearl->capacity;
    
    PearlNode **new_buckets = samrena_push(pearl->arena, sizeof(PearlNode *) * new_capacity);
    if (new_buckets == NULL) {
        pearl_set_error(pearl, PEARL_ERROR_MEMORY_EXHAUSTED);
        pearl->stats.failed_allocations++;
        return false;
    }
    
    memset(new_buckets, 0, sizeof(PearlNode *) * new_capacity);
    
    pearl->buckets = new_buckets;
    pearl->capacity = new_capacity;
    pearl->stats.resize_count++;
    
    for (size_t i = 0; i < old_capacity; i++) {
        PearlNode *current = old_buckets[i];
        while (current != NULL) {
            PearlNode *next = current->next;
            
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

bool pearl_add(Pearl *pearl, const void *element) {
    if (pearl == NULL || element == NULL) {
        if (pearl) pearl_set_error(pearl, PEARL_ERROR_NULL_PARAM);
        return false;
    }
    
    pearl->stats.total_operations++;
    
    uint32_t hash = pearl->hash(element, pearl->element_size);
    size_t bucket_index = hash % pearl->capacity;
    
    PearlNode *current = pearl->buckets[bucket_index];
    size_t chain_length = 0;
    
    while (current != NULL) {
        chain_length++;
        if (current->hash == hash && 
            pearl->equals(current->element, element, pearl->element_size)) {
            pearl_set_error(pearl, PEARL_ERROR_ELEMENT_EXISTS);
            return false;
        }
        current = current->next;
    }
    
    if (chain_length > 0) {
        pearl->stats.total_collisions++;
        if (chain_length > pearl->stats.max_chain_length) {
            pearl->stats.max_chain_length = chain_length;
        }
    }
    
    if ((float)(pearl->size + 1) / pearl->capacity > pearl->load_factor) {
        if (!pearl_resize(pearl, pearl->capacity * 2)) {
            pearl_set_error(pearl, PEARL_ERROR_RESIZE_FAILED);
            return false;
        }
        bucket_index = hash % pearl->capacity;
    }
    
    PearlNode *new_node = samrena_push(pearl->arena, sizeof(PearlNode));
    if (new_node == NULL) {
        pearl_set_error(pearl, PEARL_ERROR_MEMORY_EXHAUSTED);
        pearl->stats.failed_allocations++;
        return false;
    }
    
    new_node->element = samrena_push(pearl->arena, pearl->element_size);
    if (new_node->element == NULL) {
        pearl_set_error(pearl, PEARL_ERROR_MEMORY_EXHAUSTED);
        pearl->stats.failed_allocations++;
        return false;
    }
    
    memcpy(new_node->element, element, pearl->element_size);
    new_node->hash = hash;
    new_node->element_size = pearl->element_size;
    new_node->next = pearl->buckets[bucket_index];
    pearl->buckets[bucket_index] = new_node;
    
    pearl->size++;
    pearl_set_error(pearl, PEARL_ERROR_NONE);
    return true;
}

bool pearl_contains(const Pearl *pearl, const void *element) {
    if (pearl == NULL || element == NULL) {
        return false;
    }
    
    uint32_t hash = pearl->hash(element, pearl->element_size);
    size_t bucket_index = hash % pearl->capacity;
    
    PearlNode *current = pearl->buckets[bucket_index];
    while (current != NULL) {
        if (current->hash == hash && 
            pearl->equals(current->element, element, pearl->element_size)) {
            return true;
        }
        current = current->next;
    }
    
    return false;
}

bool pearl_remove(Pearl *pearl, const void *element) {
    if (pearl == NULL || element == NULL) {
        if (pearl) pearl_set_error(pearl, PEARL_ERROR_NULL_PARAM);
        return false;
    }
    
    pearl->stats.total_operations++;
    
    uint32_t hash = pearl->hash(element, pearl->element_size);
    size_t bucket_index = hash % pearl->capacity;
    
    PearlNode **current_ptr = &pearl->buckets[bucket_index];
    
    while (*current_ptr != NULL) {
        PearlNode *current = *current_ptr;
        if (current->hash == hash && 
            pearl->equals(current->element, element, pearl->element_size)) {
            
            *current_ptr = current->next;
            pearl->size--;
            pearl_set_error(pearl, PEARL_ERROR_NONE);
            return true;
        }
        current_ptr = &current->next;
    }
    
    pearl_set_error(pearl, PEARL_ERROR_ELEMENT_NOT_FOUND);
    return false;
}

void pearl_clear(Pearl *pearl) {
    if (pearl == NULL) return;
    
    for (size_t i = 0; i < pearl->capacity; i++) {
        pearl->buckets[i] = NULL;
    }
    
    pearl->size = 0;
    pearl_set_error(pearl, PEARL_ERROR_NONE);
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

size_t pearl_size(const Pearl *pearl) {
    if (pearl == NULL) return 0;
    return pearl->size;
}

bool pearl_is_empty(const Pearl *pearl) {
    if (pearl == NULL) return true;
    return pearl->size == 0;
}

// =============================================================================
// PERFORMANCE AND DEBUGGING
// =============================================================================

PearlStats pearl_get_stats(const Pearl *pearl) {
    PearlStats empty_stats = {0};
    if (pearl == NULL) return empty_stats;
    
    PearlStats stats = pearl->stats;
    
    if (pearl->capacity > 0 && pearl->size > 0) {
        size_t total_chain_length = 0;
        size_t non_empty_buckets = 0;
        
        for (size_t i = 0; i < pearl->capacity; i++) {
            size_t chain_length = 0;
            PearlNode *current = pearl->buckets[i];
            
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

void pearl_reset_stats(Pearl *pearl) {
    if (pearl == NULL) return;
    memset(&pearl->stats, 0, sizeof(PearlStats));
}

void pearl_print_stats(const Pearl *pearl) {
    if (pearl == NULL) return;
    
    PearlStats stats = pearl_get_stats(pearl);
    
    printf("Pearl Set Statistics:\n");
    printf("  Size: %zu elements\n", pearl->size);
    printf("  Capacity: %zu buckets\n", pearl->capacity);
    printf("  Load Factor: %.2f\n", pearl->capacity > 0 ? (double)pearl->size / pearl->capacity : 0.0);
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

void pearl_foreach(const Pearl *pearl, PearlIterator iterator, void *user_data) {
    if (pearl == NULL || iterator == NULL) return;
    
    for (size_t i = 0; i < pearl->capacity; i++) {
        PearlNode *current = pearl->buckets[i];
        while (current != NULL) {
            iterator(current->element, user_data);
            current = current->next;
        }
    }
}

// =============================================================================
// COPY FUNCTIONS
// =============================================================================

Pearl *pearl_copy(const Pearl *pearl, Samrena *samrena) {
    if (pearl == NULL || samrena == NULL) {
        return NULL;
    }
    
    Pearl *copy = pearl_create_custom(pearl->element_size, pearl->capacity, samrena,
                                     pearl->hash, pearl->equals);
    if (copy == NULL) {
        return NULL;
    }
    
    copy->load_factor = pearl->load_factor;
    copy->hash_func = pearl->hash_func;
    
    for (size_t i = 0; i < pearl->capacity; i++) {
        PearlNode *current = pearl->buckets[i];
        while (current != NULL) {
            if (!pearl_add(copy, current->element)) {
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

size_t pearl_to_array(const Pearl *pearl, void *array, size_t max_elements) {
    if (pearl == NULL || array == NULL || max_elements == 0) {
        return 0;
    }
    
    char *arr = (char *)array;
    size_t count = 0;
    
    for (size_t i = 0; i < pearl->capacity && count < max_elements; i++) {
        PearlNode *current = pearl->buckets[i];
        while (current != NULL && count < max_elements) {
            memcpy(arr + (count * pearl->element_size), current->element, pearl->element_size);
            count++;
            current = current->next;
        }
    }
    
    return count;
}

Pearl *pearl_from_array(const void *array, size_t count, size_t element_size, Samrena *samrena) {
    if (array == NULL || count == 0 || element_size == 0 || samrena == NULL) {
        return NULL;
    }
    
    size_t initial_capacity = count > PEARL_MIN_CAPACITY ? count * 2 : PEARL_MIN_CAPACITY;
    Pearl *pearl = pearl_create(element_size, initial_capacity, samrena);
    if (pearl == NULL) {
        return NULL;
    }
    
    const char *arr = (const char *)array;
    for (size_t i = 0; i < count; i++) {
        const void *element = arr + (i * element_size);
        pearl_add(pearl, element);
    }
    
    return pearl;
}
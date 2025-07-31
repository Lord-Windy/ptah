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
#include <datazoo/hash.h>
#include <string.h>

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
            return datazoo_hash_djb2;
        case PEARL_HASH_FNV1A:
            return datazoo_hash_fnv1a;
        case PEARL_HASH_MURMUR3:
            return datazoo_hash_murmur3;
        default:
            return datazoo_hash_djb2;
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
    
    pearl->hash = hash_fn ? hash_fn : datazoo_hash_djb2;
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
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

#ifndef PEARL_H
#define PEARL_H

// =============================================================================
// STANDARD INCLUDES
// =============================================================================

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// =============================================================================
// LIBRARY DEPENDENCIES
// =============================================================================

#include <samrena.h>

// =============================================================================
// PEARL - Set Data Structure for Unique Elements
// =============================================================================

// =============================================================================
// CORE ENUMERATIONS
// =============================================================================

// Error types
typedef enum {
    PEARL_ERROR_NONE = 0,
    PEARL_ERROR_NULL_PARAM,
    PEARL_ERROR_MEMORY_EXHAUSTED,
    PEARL_ERROR_RESIZE_FAILED,
    PEARL_ERROR_ELEMENT_NOT_FOUND,
    PEARL_ERROR_ELEMENT_EXISTS
} PearlError;

// Hash function type (reuse from honeycomb)
typedef enum {
    PEARL_HASH_DJB2,
    PEARL_HASH_FNV1A,
    PEARL_HASH_MURMUR3
} PearlHashFunction;

// =============================================================================
// CALLBACK FUNCTION TYPES
// =============================================================================

// Error callback function type
typedef void (*PearlErrorCallback)(PearlError error, const char *message, void *user_data);

// =============================================================================
// PERFORMANCE STRUCTURES
// =============================================================================

// Performance metrics structure
typedef struct {
    size_t total_collisions;
    size_t max_chain_length;
    size_t resize_count;
    double average_chain_length;
    size_t total_operations;
    size_t failed_allocations;
} PearlStats;

// =============================================================================
// CORE STRUCTURES
// =============================================================================

// Element node for chaining
typedef struct PearlNode {
    void *element;
    size_t element_size;
    uint32_t hash;
    struct PearlNode *next;
} PearlNode;

// Main set structure
typedef struct {
    PearlNode **buckets;
    size_t size;                    // Current number of elements
    size_t capacity;                // Number of buckets
    size_t element_size;            // Size of each element
    Samrena *arena;                 // Arena for memory allocation
    float load_factor;              // Threshold for resizing (default 0.75)
    PearlHashFunction hash_func;    // Hash function to use
    
    // Function pointers for custom operations
    uint32_t (*hash)(const void *element, size_t size);
    bool (*equals)(const void *a, const void *b, size_t size);
    
    // Statistics and error handling
    PearlStats stats;
    PearlErrorCallback error_callback;
    void *error_callback_data;
    PearlError last_error;
} Pearl;

// =============================================================================
// CORE API - Pearl Management
// =============================================================================

/**
 * Creates a new pearl set with default hash function.
 * @param element_size Size of each element in bytes
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @return New pearl instance or NULL if samrena is NULL
 */
Pearl *pearl_create(size_t element_size, size_t initial_capacity, Samrena *samrena);

/**
 * Creates a new pearl set with specified hash function.
 * @param element_size Size of each element in bytes
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @param hash_func Hash function to use
 * @return New pearl instance or NULL if samrena is NULL
 */
Pearl *pearl_create_with_hash(size_t element_size, size_t initial_capacity, 
                              Samrena *samrena, PearlHashFunction hash_func);

/**
 * Creates a new pearl set with custom hash and equality functions.
 * @param element_size Size of each element in bytes
 * @param initial_capacity Initial number of buckets
 * @param samrena Memory arena to use - REQUIRED (non-null)
 * @param hash_fn Custom hash function
 * @param equals_fn Custom equality function
 * @return New pearl instance or NULL if samrena is NULL
 */
Pearl *pearl_create_custom(size_t element_size, size_t initial_capacity, Samrena *samrena,
                          uint32_t (*hash_fn)(const void *, size_t),
                          bool (*equals_fn)(const void *, const void *, size_t));

void pearl_destroy(Pearl *pearl);

// =============================================================================
// CORE SET OPERATIONS API
// =============================================================================

bool pearl_add(Pearl *pearl, const void *element);
bool pearl_remove(Pearl *pearl, const void *element);
bool pearl_contains(const Pearl *pearl, const void *element);
void pearl_clear(Pearl *pearl);

// =============================================================================
// SET OPERATIONS API
// =============================================================================

Pearl *pearl_union(const Pearl *set1, const Pearl *set2, Samrena *samrena);
Pearl *pearl_intersection(const Pearl *set1, const Pearl *set2, Samrena *samrena);
Pearl *pearl_difference(const Pearl *set1, const Pearl *set2, Samrena *samrena);
Pearl *pearl_symmetric_difference(const Pearl *set1, const Pearl *set2, Samrena *samrena);

// =============================================================================
// SUBSET OPERATIONS API
// =============================================================================

bool pearl_is_subset(const Pearl *subset, const Pearl *superset);
bool pearl_is_superset(const Pearl *superset, const Pearl *subset);
bool pearl_is_disjoint(const Pearl *set1, const Pearl *set2);
bool pearl_equals(const Pearl *set1, const Pearl *set2);

// =============================================================================
// INFORMATION API
// =============================================================================

size_t pearl_size(const Pearl *pearl);
bool pearl_is_empty(const Pearl *pearl);

// =============================================================================
// COPY API  
// =============================================================================

Pearl *pearl_copy(const Pearl *pearl, Samrena *samrena);

// =============================================================================
// COLLECTION API
// =============================================================================

size_t pearl_to_array(const Pearl *pearl, void *array, size_t max_elements);
Pearl *pearl_from_array(const void *array, size_t count, size_t element_size, Samrena *samrena);

// =============================================================================
// ITERATOR API
// =============================================================================

typedef void (*PearlIterator)(const void *element, void *user_data);
void pearl_foreach(const Pearl *pearl, PearlIterator iterator, void *user_data);

// =============================================================================
// FUNCTIONAL PROGRAMMING API
// =============================================================================

Pearl *pearl_filter(const Pearl *pearl, bool (*predicate)(const void *, void *), 
                   void *user_data, Samrena *samrena);
Pearl *pearl_map(const Pearl *pearl, void (*transform)(const void *, void *, void *),
                size_t new_element_size, void *user_data, Samrena *samrena);

// =============================================================================
// PERFORMANCE AND DEBUGGING API
// =============================================================================

PearlStats pearl_get_stats(const Pearl *pearl);
void pearl_reset_stats(Pearl *pearl);
void pearl_print_stats(const Pearl *pearl);

// =============================================================================
// ERROR HANDLING API
// =============================================================================

void pearl_set_error_callback(Pearl *pearl, PearlErrorCallback callback, void *user_data);
PearlError pearl_get_last_error(const Pearl *pearl);
const char *pearl_error_string(PearlError error);

// =============================================================================
// TYPE-SAFE WRAPPER MACROS
// =============================================================================
#define PEARL_DEFINE_TYPED(name, type) \
    typedef struct name##_pearl { \
        Pearl *base; \
    } name##_pearl; \
    \
    static inline name##_pearl *name##_create(size_t initial_capacity, Samrena *samrena) { \
        name##_pearl *typed_set = samrena_push(samrena, sizeof(name##_pearl)); \
        if (typed_set == NULL) return NULL; \
        typed_set->base = pearl_create(sizeof(type), initial_capacity, samrena); \
        if (typed_set->base == NULL) return NULL; \
        return typed_set; \
    } \
    \
    static inline void name##_destroy(name##_pearl *p) { \
        if (p != NULL && p->base != NULL) { \
            pearl_destroy(p->base); \
        } \
    } \
    \
    static inline bool name##_add(name##_pearl *p, type element) { \
        if (p == NULL || p->base == NULL) return false; \
        return pearl_add(p->base, &element); \
    } \
    \
    static inline bool name##_remove(name##_pearl *p, type element) { \
        if (p == NULL || p->base == NULL) return false; \
        return pearl_remove(p->base, &element); \
    } \
    \
    static inline bool name##_contains(const name##_pearl *p, type element) { \
        if (p == NULL || p->base == NULL) return false; \
        return pearl_contains(p->base, &element); \
    } \
    \
    static inline void name##_clear(name##_pearl *p) { \
        if (p != NULL && p->base != NULL) { \
            pearl_clear(p->base); \
        } \
    } \
    \
    static inline size_t name##_size(const name##_pearl *p) { \
        if (p == NULL || p->base == NULL) return 0; \
        return pearl_size(p->base); \
    } \
    \
    static inline bool name##_is_empty(const name##_pearl *p) { \
        if (p == NULL || p->base == NULL) return true; \
        return pearl_is_empty(p->base); \
    } \
    \
    typedef void (*name##_iterator)(type element, void *user_data); \
    \
    static inline void name##_foreach(const name##_pearl *p, name##_iterator iterator, void *user_data) { \
        if (p == NULL || p->base == NULL || iterator == NULL) return; \
        pearl_foreach(p->base, (PearlIterator)iterator, user_data); \
    }

// =============================================================================
// COMMON TYPE-SAFE INSTANTIATIONS
// =============================================================================

PEARL_DEFINE_TYPED(int_set, int)
PEARL_DEFINE_TYPED(uint_set, unsigned int)
PEARL_DEFINE_TYPED(long_set, long)
PEARL_DEFINE_TYPED(ulong_set, unsigned long)
PEARL_DEFINE_TYPED(ptr_set, void*)

// =============================================================================
// STRING SET SPECIALIZATION
// =============================================================================

typedef struct string_set_pearl {
    Pearl *base;
} string_set_pearl;

static inline string_set_pearl *string_set_create(size_t initial_capacity, Samrena *samrena);
static inline void string_set_destroy(string_set_pearl *p);
static inline bool string_set_add(string_set_pearl *p, const char *str);
static inline bool string_set_remove(string_set_pearl *p, const char *str);
static inline bool string_set_contains(const string_set_pearl *p, const char *str);
static inline void string_set_clear(string_set_pearl *p);
static inline size_t string_set_size(const string_set_pearl *p);
static inline bool string_set_is_empty(const string_set_pearl *p);

#endif // PEARL_H
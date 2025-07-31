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
#include "samvector.h"
#include <string.h>
#include <stddef.h>

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

// Internal helper for owned arena sizing
static size_t calculate_arena_size(size_t element_size, size_t initial_capacity) {
    // Base size for initial capacity
    size_t base_size = element_size * initial_capacity;
    
    // Add growth headroom (estimate 4x total growth)
    size_t with_growth = base_size * 4;
    
    // Minimum arena size for efficiency
    const size_t min_arena_size = 4096;  // 4KB
    const size_t max_arena_size = 1024 * 1024;  // 1MB default max
    
    if (with_growth < min_arena_size) return min_arena_size;
    if (with_growth > max_arena_size) return max_arena_size;
    
    // Round up to nearest power of 2 for alignment
    size_t rounded = 1;
    while (rounded < with_growth) rounded <<= 1;
    
    return rounded;
}

static bool validate_vector_ownership(const SamrenaVector* vec, bool requires_owned) {
    if (!vec) return false;
    if (requires_owned && !vec->owns_arena) return false;
    if (!requires_owned && vec->owns_arena) return false;
    return true;
}

SamrenaVector* samrena_vector_init(Samrena* arena, uint64_t element_size, uint64_t initial_capacity) {
    if (!arena || element_size == 0) {
        return NULL;
    }
    
    SamrenaVector* vec = samrena_push_zero(arena, sizeof(SamrenaVector));
    if (!vec) {
        return NULL;
    }
    
    vec->element_size = element_size;
    vec->capacity = initial_capacity > 0 ? initial_capacity : 1; // Use minimum capacity of 1
    vec->size = 0;
    vec->arena = arena;
    vec->owns_arena = false;  // External arena by default
    vec->can_shrink = false;  // Conservative default - depends on arena implementation
    vec->growth_factor = 1.5f; // Default growth factor
    vec->min_growth = 8;      // Default minimum growth
    
    if (vec->capacity > 0) {
        vec->data = samrena_push_zero(arena, element_size * vec->capacity);
        if (!vec->data) {
            return NULL;
        }
    } else {
        vec->data = NULL;
    }
    
    return vec;
}

void* samrena_vector_push(Samrena* arena, SamrenaVector* vec, const void* element) {
    if (!arena || !vec || !element) {
        return NULL;
    }
    
    if (vec->size >= vec->capacity) {
        // Calculate new capacity based on growth strategy
        uint64_t growth = (uint64_t)(vec->capacity * vec->growth_factor);
        if (growth < vec->capacity + vec->min_growth) {
            growth = vec->capacity + vec->min_growth;
        }
        uint64_t new_capacity = growth;
        
        void* new_data = samrena_push_zero(arena, vec->element_size * new_capacity);
        if (!new_data) {
            return NULL;
        }
        
        if (vec->data && vec->size > 0) {
            memcpy(new_data, vec->data, vec->element_size * vec->size);
        }
        
        vec->data = new_data;
        vec->capacity = new_capacity;
    }
    
    void* dest = (uint8_t*)vec->data + (vec->size * vec->element_size);
    memcpy(dest, element, vec->element_size);
    vec->size++;
    
    return dest;
}

void* samrena_vector_pop(SamrenaVector* vec) {
    if (!vec || vec->size == 0) {
        return NULL;
    }
    
    vec->size--;
    return (uint8_t*)vec->data + (vec->size * vec->element_size);
}

void* samrena_vector_resize(Samrena* arena, SamrenaVector* vec, uint64_t new_capacity) {
    if (!arena || !vec) {
        return NULL;
    }
    
    if (new_capacity == vec->capacity) {
        return vec->data;
    }
    
    void* new_data = NULL;
    if (new_capacity > 0) {
        new_data = samrena_push_zero(arena, vec->element_size * new_capacity);
        if (!new_data) {
            return NULL;
        }
        
        if (vec->data && vec->size > 0) {
            uint64_t copy_count = vec->size < new_capacity ? vec->size : new_capacity;
            memcpy(new_data, vec->data, vec->element_size * copy_count);
        }
    }
    
    vec->data = new_data;
    vec->capacity = new_capacity;
    
    if (vec->size > new_capacity) {
        vec->size = new_capacity;
    }
    
    return new_data;
}

SamrenaVectorError samrena_vector_get(const SamrenaVector* vec, size_t index, void* out_element) {
    if (!vec || !out_element) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    if (index >= vec->size) {
        return SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS;
    }
    
    const uint8_t* src = (const uint8_t*)vec->data + (index * vec->element_size);
    memcpy(out_element, src, vec->element_size);
    
    return SAMRENA_VECTOR_SUCCESS;
}

SamrenaVectorError samrena_vector_set(SamrenaVector* vec, size_t index, const void* element) {
    if (!vec || !element) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    if (index >= vec->size) {
        return SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS;
    }
    
    uint8_t* dest = (uint8_t*)vec->data + (index * vec->element_size);
    memcpy(dest, element, vec->element_size);
    
    return SAMRENA_VECTOR_SUCCESS;
}

void* samrena_vector_at(SamrenaVector* vec, size_t index) {
    if (!vec || index >= vec->size) {
        return NULL;
    }
    
    return (uint8_t*)vec->data + (index * vec->element_size);
}

const void* samrena_vector_at_const(const SamrenaVector* vec, size_t index) {
    if (!vec || index >= vec->size) {
        return NULL;
    }
    
    return (const uint8_t*)vec->data + (index * vec->element_size);
}

void* samrena_vector_front(SamrenaVector* vec) {
    if (!vec || vec->size == 0) {
        return NULL;
    }
    
    return vec->data;
}

const void* samrena_vector_front_const(const SamrenaVector* vec) {
    if (!vec || vec->size == 0) {
        return NULL;
    }
    
    return vec->data;
}

void* samrena_vector_back(SamrenaVector* vec) {
    if (!vec || vec->size == 0) {
        return NULL;
    }
    
    return (uint8_t*)vec->data + ((vec->size - 1) * vec->element_size);
}

const void* samrena_vector_back_const(const SamrenaVector* vec) {
    if (!vec || vec->size == 0) {
        return NULL;
    }
    
    return (const uint8_t*)vec->data + ((vec->size - 1) * vec->element_size);
}

void* samrena_vector_data(SamrenaVector* vec) {
    if (!vec) {
        return NULL;
    }
    
    return vec->data;
}

const void* samrena_vector_data_const(const SamrenaVector* vec) {
    if (!vec) {
        return NULL;
    }
    
    return vec->data;
}

// =============================================================================
// CAPACITY CONTROL FUNCTIONS
// =============================================================================

SamrenaVectorError samrena_vector_reserve(SamrenaVector* vec, size_t min_capacity) {
    if (!vec) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    if (min_capacity <= vec->capacity) {
        return SAMRENA_VECTOR_SUCCESS; // Already have enough capacity
    }
    
    void* new_data = samrena_push_zero(vec->arena, vec->element_size * min_capacity);
    if (!new_data) {
        return SAMRENA_VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    if (vec->data && vec->size > 0) {
        memcpy(new_data, vec->data, vec->element_size * vec->size);
    }
    
    vec->data = new_data;
    vec->capacity = min_capacity;
    
    return SAMRENA_VECTOR_SUCCESS;
}

SamrenaVectorError samrena_vector_shrink_to_fit(SamrenaVector* vec) {
    if (!vec) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    if (vec->size == vec->capacity) {
        return SAMRENA_VECTOR_SUCCESS; // Already fit
    }
    
    if (vec->size == 0) {
        vec->data = NULL;
        vec->capacity = 0;
        return SAMRENA_VECTOR_SUCCESS;
    }
    
    // Note: Arena-based allocation may not support shrinking
    // This is a best-effort operation
    void* new_data = samrena_push_zero(vec->arena, vec->element_size * vec->size);
    if (!new_data) {
        return SAMRENA_VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    memcpy(new_data, vec->data, vec->element_size * vec->size);
    vec->data = new_data;
    vec->capacity = vec->size;
    
    return SAMRENA_VECTOR_SUCCESS;
}

SamrenaVectorError samrena_vector_set_capacity(SamrenaVector* vec, size_t capacity) {
    if (!vec) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    if (capacity == vec->capacity) {
        return SAMRENA_VECTOR_SUCCESS;
    }
    
    if (capacity == 0) {
        vec->data = NULL;
        vec->capacity = 0;
        vec->size = 0;
        return SAMRENA_VECTOR_SUCCESS;
    }
    
    void* new_data = samrena_push_zero(vec->arena, vec->element_size * capacity);
    if (!new_data) {
        return SAMRENA_VECTOR_ERROR_ALLOCATION_FAILED;
    }
    
    if (vec->data && vec->size > 0) {
        size_t copy_count = vec->size < capacity ? vec->size : capacity;
        memcpy(new_data, vec->data, vec->element_size * copy_count);
    }
    
    vec->data = new_data;
    vec->capacity = capacity;
    
    if (vec->size > capacity) {
        vec->size = capacity;
    }
    
    return SAMRENA_VECTOR_SUCCESS;
}

// =============================================================================
// CONTENT MANAGEMENT FUNCTIONS
// =============================================================================

void samrena_vector_clear(SamrenaVector* vec) {
    if (vec) {
        vec->size = 0;
    }
}

SamrenaVectorError samrena_vector_truncate(SamrenaVector* vec, size_t new_size) {
    if (!vec) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    if (new_size > vec->size) {
        return SAMRENA_VECTOR_ERROR_OUT_OF_BOUNDS;
    }
    
    vec->size = new_size;
    return SAMRENA_VECTOR_SUCCESS;
}

SamrenaVectorError samrena_vector_reset(SamrenaVector* vec, size_t initial_capacity) {
    if (!vec) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    vec->size = 0;
    
    if (initial_capacity != vec->capacity) {
        return samrena_vector_set_capacity(vec, initial_capacity);
    }
    
    return SAMRENA_VECTOR_SUCCESS;
}

// =============================================================================
// QUERY FUNCTIONS
// =============================================================================

size_t samrena_vector_size(const SamrenaVector* vec) {
    return vec ? vec->size : 0;
}

size_t samrena_vector_capacity(const SamrenaVector* vec) {
    return vec ? vec->capacity : 0;
}

bool samrena_vector_is_empty(const SamrenaVector* vec) {
    return !vec || vec->size == 0;
}

bool samrena_vector_is_full(const SamrenaVector* vec) {
    return vec && vec->size >= vec->capacity;
}

size_t samrena_vector_available(const SamrenaVector* vec) {
    return vec && vec->capacity > vec->size ? vec->capacity - vec->size : 0;
}

// =============================================================================
// GROWTH CONTROL FUNCTIONS
// =============================================================================

void samrena_vector_set_growth_factor(SamrenaVector* vec, float factor) {
    if (vec && factor > 1.0f) {
        vec->growth_factor = factor;
    }
}

void samrena_vector_set_min_growth(SamrenaVector* vec, size_t min_elements) {
    if (vec) {
        vec->min_growth = min_elements;
    }
}

SamrenaVectorStats samrena_vector_get_stats(const SamrenaVector* vec) {
    SamrenaVectorStats stats = {0};
    
    if (vec) {
        stats.used_bytes = vec->size * vec->element_size;
        stats.allocated_bytes = vec->capacity * vec->element_size;
        stats.wasted_bytes = stats.allocated_bytes - stats.used_bytes;
        stats.utilization = stats.allocated_bytes > 0 ? 
            (float)stats.used_bytes / (float)stats.allocated_bytes : 0.0f;
    }
    
    return stats;
}

// =============================================================================
// MEMORY OWNERSHIP FUNCTIONS
// =============================================================================

SamrenaVector* samrena_vector_init_owned(uint64_t element_size, uint64_t initial_capacity) {
    if (element_size == 0) return NULL;
    
    // Calculate and create appropriately sized arena
    size_t arena_size = calculate_arena_size(element_size, initial_capacity);
    size_t pages = (arena_size + 4095) / 4096;
    if (pages < 1) pages = 1;
    
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = pages;
    Samrena* arena = samrena_create(&config);
    if (!arena) return NULL;
    
    // Initialize vector with owned arena
    SamrenaVector* vec = samrena_vector_init_with_arena(arena, element_size, initial_capacity);
    if (!vec) {
        samrena_destroy(arena);
        return NULL;
    }
    
    // Mark arena as owned
    vec->owns_arena = true;
    vec->can_shrink = true;  // Owned arenas can be resized
    
    return vec;
}

SamrenaVector* samrena_vector_init_with_arena(Samrena* arena, uint64_t element_size, uint64_t initial_capacity) {
    if (!arena || element_size == 0) return NULL;
    
    // Allocate vector structure from arena
    SamrenaVector* vec = samrena_push_zero(arena, sizeof(SamrenaVector));
    if (!vec) return NULL;
    
    // Initialize vector fields
    vec->size = 0;
    vec->element_size = element_size;
    vec->capacity = initial_capacity > 0 ? initial_capacity : 1;
    vec->arena = arena;
    vec->owns_arena = false;
    vec->growth_factor = 1.5f;
    vec->min_growth = 8;
    vec->can_shrink = false;  // Conservative default for shared arenas
    
    // Allocate initial data buffer
    if (vec->capacity > 0) {
        vec->data = samrena_push_zero(arena, vec->capacity * element_size);
        if (!vec->data) {
            // Don't free vec - it's in the arena
            return NULL;
        }
    } else {
        vec->data = NULL;
    }
    
    return vec;
}

void samrena_vector_destroy(SamrenaVector* vec) {
    if (!vec) return;
    
    if (vec->owns_arena) {
        // Destroy the entire arena (includes vector and data)
        samrena_destroy(vec->arena);
    } else {
        // Vector allocated in external arena - just mark as invalid
        // Note: Can't actually free from arena-based allocation
        // External arena owner is responsible for cleanup
        vec->data = NULL;
        vec->size = 0;
        vec->capacity = 0;
        vec->arena = NULL;
    }
}

// Alternative for shared arena vectors - explicit cleanup
void samrena_vector_cleanup(SamrenaVector* vec) {
    if (!vec || vec->owns_arena) return;
    
    // Clear vector but don't attempt to free arena memory
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

// =============================================================================
// OWNED VECTOR OPERATIONS
// =============================================================================

void* samrena_vector_push_owned(SamrenaVector* vec, const void* element) {
    if (!vec || !vec->owns_arena) return NULL;
    return samrena_vector_push_auto(vec, element);
}

SamrenaVectorError samrena_vector_reserve_owned(SamrenaVector* vec, size_t min_capacity) {
    if (!vec || !vec->owns_arena) return SAMRENA_VECTOR_ERROR_INVALID_OPERATION;
    return samrena_vector_reserve_auto(vec, min_capacity);
}

// =============================================================================
// AUTO-DETECTION FUNCTIONS
// =============================================================================

void* samrena_vector_push_auto(SamrenaVector* vec, const void* element) {
    if (!vec || !element) return NULL;
    
    // Use the vector's arena regardless of ownership
    return samrena_vector_push_with_arena(vec->arena, vec, element);
}

SamrenaVectorError samrena_vector_reserve_auto(SamrenaVector* vec, size_t min_capacity) {
    if (!vec) return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    
    // Check if growth is needed
    if (min_capacity <= vec->capacity) return SAMRENA_VECTOR_SUCCESS;
    
    // Use vector's arena for reallocation
    return samrena_vector_reserve(vec, min_capacity);
}

void* samrena_vector_push_with_arena(Samrena* arena, SamrenaVector* vec, const void* element) {
    // Explicit arena override - useful for migration or special cases
    return samrena_vector_push(arena, vec, element);
}

// =============================================================================
// OWNERSHIP QUERY FUNCTIONS
// =============================================================================

bool samrena_vector_owns_arena(const SamrenaVector* vec) {
    return vec && vec->owns_arena;
}

Samrena* samrena_vector_get_arena(const SamrenaVector* vec) {
    return vec ? vec->arena : NULL;
}

bool samrena_vector_can_grow(const SamrenaVector* vec) {
    return vec && vec->arena;  // Simplified - arena space checking would need arena API
}

// =============================================================================
// MIGRATION HELPERS
// =============================================================================

// Convert shared vector to owned (creates new arena and copies data)
SamrenaVector* samrena_vector_make_owned(const SamrenaVector* shared_vec) {
    if (!shared_vec || shared_vec->owns_arena) return NULL;
    
    // Create new owned vector with same configuration
    SamrenaVector* owned = samrena_vector_init_owned(
        shared_vec->element_size, 
        shared_vec->capacity
    );
    if (!owned) return NULL;
    
    // Copy configuration
    owned->growth_factor = shared_vec->growth_factor;
    owned->min_growth = shared_vec->min_growth;
    
    // Copy data
    if (shared_vec->size > 0) {
        memcpy(owned->data, shared_vec->data, 
               shared_vec->size * shared_vec->element_size);
        owned->size = shared_vec->size;
    }
    
    return owned;
}


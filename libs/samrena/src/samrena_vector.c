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

static size_t calculate_arena_size(size_t element_size, size_t initial_capacity) {
    size_t base_size = element_size * initial_capacity;
    size_t with_growth = base_size * 4;
    
    const size_t min_arena_size = 4096;
    const size_t max_arena_size = 1024 * 1024;
    
    if (with_growth < min_arena_size) return min_arena_size;
    if (with_growth > max_arena_size) return max_arena_size;
    
    size_t rounded = 1;
    while (rounded < with_growth) rounded <<= 1;
    
    return rounded;
}

// =============================================================================
// CORE VECTOR API
// =============================================================================

SamrenaVector* samrena_vector_init(Samrena* arena, uint64_t element_size, uint64_t initial_capacity) {
    if (!arena || element_size == 0) {
        return NULL;
    }
    
    SamrenaVector* vec = samrena_push_zero(arena, sizeof(SamrenaVector));
    if (!vec) {
        return NULL;
    }
    
    vec->element_size = element_size;
    vec->capacity = initial_capacity > 0 ? initial_capacity : 1;
    vec->size = 0;
    vec->arena = arena;
    vec->owns_arena = false;
    vec->growth_factor = 1.5f;
    vec->min_growth = 8;
    
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

SamrenaVector* samrena_vector_init_owned(uint64_t element_size, uint64_t initial_capacity) {
    if (element_size == 0) return NULL;
    
    size_t arena_size = calculate_arena_size(element_size, initial_capacity);
    size_t pages = (arena_size + 4095) / 4096;
    if (pages < 1) pages = 1;
    
    SamrenaConfig config = samrena_default_config();
    config.initial_pages = pages;
    Samrena* arena = samrena_create(&config);
    if (!arena) return NULL;
    
    SamrenaVector* vec = samrena_vector_init(arena, element_size, initial_capacity);
    if (!vec) {
        samrena_destroy(arena);
        return NULL;
    }
    
    vec->owns_arena = true;
    
    return vec;
}

void* samrena_vector_push(SamrenaVector* vec, const void* element) {
    if (!vec || !element) {
        return NULL;
    }
    
    if (vec->size >= vec->capacity) {
        uint64_t growth = (uint64_t)(vec->capacity * vec->growth_factor);
        if (growth < vec->capacity + vec->min_growth) {
            growth = vec->capacity + vec->min_growth;
        }
        uint64_t new_capacity = growth;
        
        void* new_data = samrena_push_zero(vec->arena, vec->element_size * new_capacity);
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

SamrenaVectorError samrena_vector_resize(SamrenaVector* vec, uint64_t new_capacity) {
    if (!vec) {
        return SAMRENA_VECTOR_ERROR_NULL_POINTER;
    }
    
    if (new_capacity == vec->capacity) {
        return SAMRENA_VECTOR_SUCCESS;
    }
    
    void* new_data = NULL;
    if (new_capacity > 0) {
        new_data = samrena_push_zero(vec->arena, vec->element_size * new_capacity);
        if (!new_data) {
            return SAMRENA_VECTOR_ERROR_ALLOCATION_FAILED;
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
    
    return SAMRENA_VECTOR_SUCCESS;
}

// =============================================================================
// ELEMENT ACCESS API
// =============================================================================

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

// =============================================================================
// CAPACITY MANAGEMENT API
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
        return samrena_vector_resize(vec, initial_capacity);
    }
    
    return SAMRENA_VECTOR_SUCCESS;
}

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

void samrena_vector_destroy(SamrenaVector* vec) {
    if (!vec) return;
    
    if (vec->owns_arena) {
        samrena_destroy(vec->arena);
    } else {
        vec->data = NULL;
        vec->size = 0;
        vec->capacity = 0;
        vec->arena = NULL;
    }
}

// =============================================================================
// ITERATOR API IMPLEMENTATION
// =============================================================================

SamrenaVectorIterator samrena_vector_iter_begin(const SamrenaVector* vec) {
    SamrenaVectorIterator iter = {0};
    if (vec && vec->size > 0) {
        iter.vector = vec;
        iter.current_index = 0;
        iter.is_valid = true;
    } else {
        iter.vector = vec;
        iter.current_index = 0;
        iter.is_valid = false;
    }
    return iter;
}

bool samrena_vector_iter_has_next(const SamrenaVectorIterator* iter) {
    return iter && iter->is_valid && iter->vector && iter->current_index < iter->vector->size;
}

const void* samrena_vector_iter_next(SamrenaVectorIterator* iter) {
    if (!samrena_vector_iter_has_next(iter)) {
        return NULL;
    }
    
    const void* element = samrena_vector_at_unchecked_const(iter->vector, iter->current_index);
    iter->current_index++;
    
    if (iter->current_index >= iter->vector->size) {
        iter->is_valid = false;
    }
    
    return element;
}

void samrena_vector_iter_reset(SamrenaVectorIterator* iter) {
    if (!iter || !iter->vector) return;
    
    iter->current_index = 0;
    iter->is_valid = (iter->vector->size > 0);
}

// =============================================================================
// FUNCTIONAL PROGRAMMING API IMPLEMENTATION
// =============================================================================

SamrenaVector* samrena_vector_filter(const SamrenaVector* vec, SamrenaVectorPredicate predicate, void* user_data, Samrena* target_arena) {
    if (!vec || !predicate || !target_arena) {
        return NULL;
    }
    
    SamrenaVector* result = samrena_vector_init(target_arena, vec->element_size, vec->size / 4 + 1);
    if (!result) {
        return NULL;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = samrena_vector_at_unchecked_const(vec, i);
        if (predicate(element, user_data)) {
            if (!samrena_vector_push(result, element)) {
                return NULL;
            }
        }
    }
    
    return result;
}

SamrenaVector* samrena_vector_map(const SamrenaVector* src_vec, size_t dst_element_size, SamrenaVectorTransform transform, void* user_data, Samrena* target_arena) {
    if (!src_vec || !transform || !target_arena || dst_element_size == 0) {
        return NULL;
    }
    
    SamrenaVector* result = samrena_vector_init(target_arena, dst_element_size, src_vec->size);
    if (!result) {
        return NULL;
    }
    
    void* temp_dst = samrena_push_zero(target_arena, dst_element_size);
    if (!temp_dst) {
        return NULL;
    }
    
    for (size_t i = 0; i < src_vec->size; i++) {
        const void* src_element = samrena_vector_at_unchecked_const(src_vec, i);
        transform(src_element, temp_dst, user_data);
        
        if (!samrena_vector_push(result, temp_dst)) {
            return NULL;
        }
    }
    
    return result;
}


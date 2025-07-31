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
        uint64_t new_capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;
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

void* samrena_vector_at_unchecked(SamrenaVector* vec, size_t index) {
    return (uint8_t*)vec->data + (index * vec->element_size);
}

const void* samrena_vector_at_unchecked_const(const SamrenaVector* vec, size_t index) {
    return (const uint8_t*)vec->data + (index * vec->element_size);
}
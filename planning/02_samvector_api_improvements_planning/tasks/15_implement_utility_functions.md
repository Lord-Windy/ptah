# Task 15: Implement Utility Functions

## Objective
Implement essential utility functions for Samvector that provide common operations like swapping, reversing, copying, and basic vector manipulation to complete the Phase 2 enhanced operations.

## Dependencies
- Task 02: Implement Element Access Functions (for safe element access)
- Task 08: Implement Error Handling (for consistent error reporting)
- Task 10: Implement Bulk Operations (for efficient copying)
- Task 14: Implement Iteration (for traversal operations)

## Implementation Plan

### 1. Element Swapping Operations
```c
// samrena_vector_utilities.c - Utility function implementations

int samrena_vector_swap(SamrenaVector* vec, size_t index1, size_t index2) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    // Check bounds for both indices
    if (index1 >= vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "First index out of bounds", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    if (index2 >= vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "Second index out of bounds", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    // No-op if swapping element with itself
    if (index1 == index2) return SAMRENA_SUCCESS;
    
    // Get pointers to elements
    char* element1 = (char*)vec->data + (index1 * vec->element_size);
    char* element2 = (char*)vec->data + (index2 * vec->element_size);
    
    // Use stack buffer for small elements, heap for large ones
    if (vec->element_size <= 256) {
        char temp_buffer[256];
        memcpy(temp_buffer, element1, vec->element_size);
        memcpy(element1, element2, vec->element_size);
        memcpy(element2, temp_buffer, vec->element_size);
    } else {
        // For large elements, use dynamic allocation
        void* temp_buffer = samrena_alloc(vec->arena, vec->element_size);
        if (!temp_buffer) {
            SAMRENA_SET_ERROR(SAMRENA_ERROR_ALLOCATION_FAILED, 
                             "Failed to allocate swap buffer", vec);
            return SAMRENA_ERROR_ALLOCATION_FAILED;
        }
        
        memcpy(temp_buffer, element1, vec->element_size);
        memcpy(element1, element2, vec->element_size);
        memcpy(element2, temp_buffer, vec->element_size);
        
        // Note: Can't free from arena in general case, 
        // but temp allocation is small and will be reclaimed
    }
    
    return SAMRENA_SUCCESS;
}

// Swap elements using indices from an index array (for sorting algorithms)
int samrena_vector_swap_indices(SamrenaVector* vec, const size_t* indices, 
                               size_t index1, size_t index2) {
    if (!indices) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Indices array is NULL", indices);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    return samrena_vector_swap(vec, indices[index1], indices[index2]);
}
```

### 2. Vector Reversal
```c
int samrena_vector_reverse(SamrenaVector* vec) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (vec->size <= 1) return SAMRENA_SUCCESS;  // Nothing to reverse
    
    size_t left = 0;
    size_t right = vec->size - 1;
    
    // Swap elements from outside to inside
    while (left < right) {
        int swap_result = samrena_vector_swap(vec, left, right);
        if (swap_result != SAMRENA_SUCCESS) return swap_result;
        
        left++;
        right--;
    }
    
    return SAMRENA_SUCCESS;
}

// Reverse a specific range within the vector
int samrena_vector_reverse_range(SamrenaVector* vec, size_t start, size_t end) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (start >= vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "Start index out of bounds", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    // Clamp end to vector size
    if (end > vec->size) end = vec->size;
    if (start >= end) return SAMRENA_SUCCESS;  // Empty range
    
    size_t left = start;
    size_t right = end - 1;
    
    while (left < right) {
        int swap_result = samrena_vector_swap(vec, left, right);
        if (swap_result != SAMRENA_SUCCESS) return swap_result;
        
        left++;
        right--;
    }
    
    return SAMRENA_SUCCESS;
}
```

### 3. Vector Copying Operations
```c
SamrenaVector* samrena_vector_copy(const SamrenaVector* src) {
    if (validate_vector(src) != SAMRENA_SUCCESS) {
        return NULL;
    }
    
    // Create new vector with same configuration
    SamrenaVector* dest = samrena_vector_init_owned(src->element_size, src->capacity);
    if (!dest) return NULL;
    
    // Copy configuration settings
    dest->growth_factor = src->growth_factor;
    dest->min_growth = src->min_growth;
    
    // Copy data if source has elements
    if (src->size > 0) {
        if (samrena_vector_push_array(dest, src->data, src->size) != SAMRENA_SUCCESS) {
            samrena_vector_destroy(dest);
            return NULL;
        }
    }
    
    return dest;
}

SamrenaVector* samrena_vector_copy_to_arena(Samrena* arena, const SamrenaVector* src) {
    if (!arena) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Arena is NULL", arena);
        return NULL;
    }
    
    if (validate_vector(src) != SAMRENA_SUCCESS) {
        return NULL;
    }
    
    // Create vector in specified arena
    SamrenaVector* dest = samrena_vector_init_with_arena(arena, src->element_size, 
                                                        src->capacity);
    if (!dest) return NULL;
    
    // Copy configuration
    dest->growth_factor = src->growth_factor;
    dest->min_growth = src->min_growth;
    
    // Copy data
    if (src->size > 0) {
        if (samrena_vector_push_array(dest, src->data, src->size) != SAMRENA_SUCCESS) {
            // Can't free from arena, but return NULL to indicate failure
            return NULL;
        }
    }
    
    return dest;
}

// Create a shallow copy (share data, separate metadata)
SamrenaVector* samrena_vector_clone_shallow(const SamrenaVector* src) {
    if (validate_vector(src) != SAMRENA_SUCCESS) {
        return NULL;
    }
    
    // Allocate vector structure from same arena
    SamrenaVector* clone = samrena_alloc(src->arena, sizeof(SamrenaVector));
    if (!clone) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_ALLOCATION_FAILED, 
                         "Failed to allocate clone", src);
        return NULL;
    }
    
    // Copy all fields (shares data pointer)
    *clone = *src;
    
    // Mark as not owning arena to prevent double-free
    clone->owns_arena = false;
    
    return clone;
}
```

### 4. Vector Resizing with Fill Values
```c
int samrena_vector_resize_with_value(SamrenaVector* vec, size_t new_size, 
                                    const void* fill_value) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (new_size == vec->size) return SAMRENA_SUCCESS;
    
    if (new_size < vec->size) {
        // Shrinking - just update size
        vec->size = new_size;
        return SAMRENA_SUCCESS;
    }
    
    // Growing - need to fill new elements
    size_t elements_to_add = new_size - vec->size;
    
    // Ensure sufficient capacity
    if (new_size > vec->capacity) {
        size_t new_capacity = calculate_bulk_capacity(vec, elements_to_add);
        err = samrena_vector_reserve_auto(vec, new_capacity);
        if (err != SAMRENA_SUCCESS) return err;
    }
    
    // Fill new elements
    if (fill_value) {
        // Fill with provided value
        char* dest = (char*)vec->data + (vec->size * vec->element_size);
        for (size_t i = 0; i < elements_to_add; i++) {
            memcpy(dest, fill_value, vec->element_size);
            dest += vec->element_size;
        }
    } else {
        // Fill with zeros
        char* dest = (char*)vec->data + (vec->size * vec->element_size);
        memset(dest, 0, elements_to_add * vec->element_size);
    }
    
    vec->size = new_size;
    return SAMRENA_SUCCESS;
}

int samrena_vector_resize(SamrenaVector* vec, size_t new_size) {
    return samrena_vector_resize_with_value(vec, new_size, NULL);  // Fill with zeros
}
```

### 5. Vector Comparison Functions
```c
bool samrena_vector_equals(const SamrenaVector* vec1, const SamrenaVector* vec2,
                          SamrenaCompareFn compare_fn, void* user_data) {
    // Handle NULL cases
    if (!vec1 && !vec2) return true;
    if (!vec1 || !vec2) return false;
    
    // Validate both vectors
    if (validate_vector(vec1) != SAMRENA_SUCCESS || 
        validate_vector(vec2) != SAMRENA_SUCCESS) {
        return false;
    }
    
    // Check basic compatibility
    if (vec1->size != vec2->size || vec1->element_size != vec2->element_size) {
        return false;
    }
    
    // Empty vectors are equal
    if (vec1->size == 0) return true;
    
    // Compare elements
    if (compare_fn) {
        // Use custom comparator
        for (size_t i = 0; i < vec1->size; i++) {
            const void* elem1 = iter_get_element_const(vec1, i);
            const void* elem2 = iter_get_element_const(vec2, i);
            
            if (compare_fn(elem1, elem2, user_data) != 0) {
                return false;
            }
        }
    } else {
        // Use byte-wise comparison
        return memcmp(vec1->data, vec2->data, vec1->size * vec1->element_size) == 0;
    }
    
    return true;
}

// Lexicographic comparison
int samrena_vector_compare(const SamrenaVector* vec1, const SamrenaVector* vec2,
                          SamrenaCompareFn compare_fn, void* user_data) {
    // Handle NULL cases
    if (!vec1 && !vec2) return 0;
    if (!vec1) return -1;
    if (!vec2) return 1;
    
    if (validate_vector(vec1) != SAMRENA_SUCCESS || 
        validate_vector(vec2) != SAMRENA_SUCCESS) {
        return 0;  // Can't compare invalid vectors
    }
    
    if (vec1->element_size != vec2->element_size) {
        return 0;  // Can't compare different element types
    }
    
    if (!compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Compare function is NULL", compare_fn);
        return 0;
    }
    
    size_t min_size = (vec1->size < vec2->size) ? vec1->size : vec2->size;
    
    // Compare common elements
    for (size_t i = 0; i < min_size; i++) {
        const void* elem1 = iter_get_element_const(vec1, i);
        const void* elem2 = iter_get_element_const(vec2, i);
        
        int cmp = compare_fn(elem1, elem2, user_data);
        if (cmp != 0) return cmp;
    }
    
    // If all common elements are equal, compare sizes
    if (vec1->size < vec2->size) return -1;
    if (vec1->size > vec2->size) return 1;
    return 0;
}
```

### 6. Vector Statistics and Information
```c
typedef struct {
    size_t size;
    size_t capacity;
    size_t element_size;
    size_t memory_used;
    size_t memory_allocated;
    size_t memory_wasted;
    float utilization;
    float growth_factor;
    size_t min_growth;
    bool owns_arena;
    bool can_shrink;
} SamrenaVectorInfo;

SamrenaVectorInfo samrena_vector_get_info(const SamrenaVector* vec) {
    SamrenaVectorInfo info = {0};
    
    if (validate_vector(vec) == SAMRENA_SUCCESS) {
        info.size = vec->size;
        info.capacity = vec->capacity;
        info.element_size = vec->element_size;
        info.memory_used = vec->size * vec->element_size;
        info.memory_allocated = vec->capacity * vec->element_size;
        info.memory_wasted = info.memory_allocated - info.memory_used;
        info.utilization = info.memory_allocated > 0 ? 
            (float)info.memory_used / info.memory_allocated : 0.0f;
        info.growth_factor = vec->growth_factor;
        info.min_growth = vec->min_growth;
        info.owns_arena = vec->owns_arena;
        info.can_shrink = vec->can_shrink;
    }
    
    return info;
}

// Print vector information for debugging
void samrena_vector_print_info(const SamrenaVector* vec, FILE* output) {
    if (!output) output = stdout;
    
    SamrenaVectorInfo info = samrena_vector_get_info(vec);
    
    fprintf(output, "SamrenaVector Info:\n");
    fprintf(output, "  Size: %zu elements\n", info.size);
    fprintf(output, "  Capacity: %zu elements\n", info.capacity);
    fprintf(output, "  Element Size: %zu bytes\n", info.element_size);
    fprintf(output, "  Memory Used: %zu bytes\n", info.memory_used);
    fprintf(output, "  Memory Allocated: %zu bytes\n", info.memory_allocated);
    fprintf(output, "  Memory Wasted: %zu bytes\n", info.memory_wasted);
    fprintf(output, "  Utilization: %.2f%%\n", info.utilization * 100.0f);
    fprintf(output, "  Growth Factor: %.2f\n", info.growth_factor);
    fprintf(output, "  Min Growth: %zu elements\n", info.min_growth);
    fprintf(output, "  Owns Arena: %s\n", info.owns_arena ? "Yes" : "No");
    fprintf(output, "  Can Shrink: %s\n", info.can_shrink ? "Yes" : "No");
}
```

### 7. Vector Validation and Integrity Checking
```c
typedef enum {
    SAMRENA_VECTOR_INTEGRITY_OK = 0,
    SAMRENA_VECTOR_INTEGRITY_NULL_POINTER = 1,
    SAMRENA_VECTOR_INTEGRITY_INVALID_SIZE = 2,
    SAMRENA_VECTOR_INTEGRITY_INVALID_CAPACITY = 4,
    SAMRENA_VECTOR_INTEGRITY_INVALID_ELEMENT_SIZE = 8,
    SAMRENA_VECTOR_INTEGRITY_NULL_DATA = 16,
    SAMRENA_VECTOR_INTEGRITY_NULL_ARENA = 32,
    SAMRENA_VECTOR_INTEGRITY_SIZE_EXCEEDS_CAPACITY = 64
} SamrenaVectorIntegrityFlags;

int samrena_vector_check_integrity(const SamrenaVector* vec) {
    int flags = SAMRENA_VECTOR_INTEGRITY_OK;
    
    // Check for NULL pointer
    if (!vec) {
        return SAMRENA_VECTOR_INTEGRITY_NULL_POINTER;
    }
    
    // Check element size
    if (vec->element_size == 0) {
        flags |= SAMRENA_VECTOR_INTEGRITY_INVALID_ELEMENT_SIZE;
    }
    
    // Check arena pointer
    if (!vec->arena) {
        flags |= SAMRENA_VECTOR_INTEGRITY_NULL_ARENA;
    }
    
    // Check size vs capacity
    if (vec->size > vec->capacity) {
        flags |= SAMRENA_VECTOR_INTEGRITY_SIZE_EXCEEDS_CAPACITY;
    }
    
    // Check data pointer consistency
    if (vec->capacity > 0 && !vec->data) {
        flags |= SAMRENA_VECTOR_INTEGRITY_NULL_DATA;
    }
    
    return flags;
}

bool samrena_vector_is_valid(const SamrenaVector* vec) {
    return samrena_vector_check_integrity(vec) == SAMRENA_VECTOR_INTEGRITY_OK;
}
```

## Performance Optimizations

### SIMD-Optimized Operations
```c
#ifdef SAMRENA_USE_SIMD
// SIMD-optimized memory operations for large vectors
static void optimized_reverse_bytes(void* data, size_t count, size_t element_size) {
    if (element_size == sizeof(int) && count >= 8) {
        // Use SIMD for int reversal
        // Implementation would depend on target architecture
    }
    
    // Fallback to standard implementation
    // ... standard reverse implementation
}
#endif
```

### Type-Specific Optimizations
```c
// Optimized swap for simple types (no temporary allocation needed)
static inline int swap_simple_type(void* a, void* b, size_t size) {
    switch (size) {
        case 1: {
            uint8_t temp = *(uint8_t*)a;
            *(uint8_t*)a = *(uint8_t*)b;
            *(uint8_t*)b = temp;
            break;
        }
        case 4: {
            uint32_t temp = *(uint32_t*)a;
            *(uint32_t*)a = *(uint32_t*)b;
            *(uint32_t*)b = temp;
            break;
        }
        case 8: {
            uint64_t temp = *(uint64_t*)a;
            *(uint64_t*)a = *(uint64_t*)b;
            *(uint64_t*)b = temp;
            break;
        }
        default:
            return 0;  // Use generic implementation
    }
    return 1;  // Optimized path used
}
```

## Testing Strategy

### Unit Tests
```c
void test_vector_swap() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 5);
    int values[] = {1, 2, 3, 4, 5};
    samrena_vector_push_array(vec, values, 5);
    
    // Test normal swap
    assert(samrena_vector_swap(vec, 0, 4) == SAMRENA_SUCCESS);
    assert(*(int*)samrena_vector_at(vec, 0) == 5);
    assert(*(int*)samrena_vector_at(vec, 4) == 1);
    
    // Test edge cases
    assert(samrena_vector_swap(vec, 2, 2) == SAMRENA_SUCCESS);  // Self-swap
    assert(samrena_vector_swap(vec, 0, 10) == SAMRENA_ERROR_OUT_OF_BOUNDS);
    
    samrena_vector_destroy(vec);
}

void test_vector_reverse() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 5);
    int values[] = {1, 2, 3, 4, 5};
    samrena_vector_push_array(vec, values, 5);
    
    assert(samrena_vector_reverse(vec) == SAMRENA_SUCCESS);
    
    int expected[] = {5, 4, 3, 2, 1};
    for (size_t i = 0; i < 5; i++) {
        assert(*(int*)samrena_vector_at(vec, i) == expected[i]);
    }
    
    samrena_vector_destroy(vec);
}

void test_vector_copy() {
    SamrenaVector* original = create_test_vector();
    SamrenaVector* copy = samrena_vector_copy(original);
    
    assert(copy != NULL);
    assert(samrena_vector_equals(original, copy, NULL, NULL));
    
    // Modify copy - should not affect original
    int new_value = 999;
    samrena_vector_push_auto(copy, &new_value);
    assert(!samrena_vector_equals(original, copy, NULL, NULL));
    
    samrena_vector_destroy(original);
    samrena_vector_destroy(copy);
}
```

### Performance Benchmarks
- Swap performance vs manual implementation
- Reverse performance for different vector sizes
- Copy performance vs manual copying
- Memory usage efficiency of utility operations

## Integration Notes
- Uses existing vector validation and error handling
- Leverages bulk operations for efficient copying
- Compatible with both owned and shared arena vectors
- Prepares utilities needed for sorting and advanced algorithms
- Maintains consistent API patterns with other vector operations
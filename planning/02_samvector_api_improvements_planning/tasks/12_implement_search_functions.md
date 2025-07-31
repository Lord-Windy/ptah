# Task 12: Implement Search Functions

## Objective
Implement the search interface designed in Task 11, providing efficient and robust search capabilities for vectors with both generic and type-specific optimizations.

## Dependencies
- Task 11: Design Search Interface (must be completed)
- Vector validation functions from Task 08
- Element access functions from Task 02

## Implementation Plan

### 1. Core Search Infrastructure
```c
// samrena_vector_search.c - Core search implementation

#include <string.h>
#include <float.h>
#include <limits.h>

// Constants for search results
#define SAMRENA_NOT_FOUND SIZE_MAX

// Internal helper for safe element access during search
static inline const void* search_get_element(const SamrenaVector* vec, size_t index) {
    return (const char*)vec->data + (index * vec->element_size);
}

// Validate search parameters
static SamrenaError validate_search_params(const SamrenaVector* vec, const void* target) {
    if (!vec) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Vector is NULL", vec);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (!target) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Target is NULL", target);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    return validate_vector(vec);
}
```

### 2. Linear Search Implementation
```c
size_t samrena_vector_find(const SamrenaVector* vec, const void* target, 
                          SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (!compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Compare function is NULL", compare_fn);
        return SAMRENA_NOT_FOUND;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = search_get_element(vec, i);
        if (compare_fn(element, target, user_data) == 0) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

size_t samrena_vector_find_if(const SamrenaVector* vec, SamrenaPredicateFn predicate, 
                             void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (!predicate) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Predicate is NULL", predicate);
        return SAMRENA_NOT_FOUND;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = search_get_element(vec, i);
        if (predicate(element, user_data)) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

size_t samrena_vector_find_last(const SamrenaVector* vec, const void* target,
                               SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return SAMRENA_NOT_FOUND;
    }
    
    // Search backwards from end
    for (size_t i = vec->size; i > 0; i--) {
        const void* element = search_get_element(vec, i - 1);
        if (compare_fn(element, target, user_data) == 0) {
            return i - 1;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

size_t samrena_vector_find_all(const SamrenaVector* vec, const void* target,
                              SamrenaCompareFn compare_fn, void* user_data,
                              size_t* indices, size_t max_indices) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return 0;
    }
    
    if (!indices && max_indices > 0) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Indices array is NULL", indices);
        return 0;
    }
    
    size_t found_count = 0;
    
    for (size_t i = 0; i < vec->size && found_count < max_indices; i++) {
        const void* element = search_get_element(vec, i);
        if (compare_fn(element, target, user_data) == 0) {
            indices[found_count] = i;
            found_count++;
        }
    }
    
    return found_count;
}
```

### 3. Existence Checking Implementation
```c
bool samrena_vector_contains(const SamrenaVector* vec, const void* target,
                            SamrenaCompareFn compare_fn, void* user_data) {
    return samrena_vector_find(vec, target, compare_fn, user_data) != SAMRENA_NOT_FOUND;
}

bool samrena_vector_any(const SamrenaVector* vec, SamrenaPredicateFn predicate,
                       void* user_data) {
    return samrena_vector_find_if(vec, predicate, user_data) != SAMRENA_NOT_FOUND;
}

bool samrena_vector_all(const SamrenaVector* vec, SamrenaPredicateFn predicate,
                       void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !predicate) {
        return false;
    }
    
    // Empty vector: vacuously true
    if (vec->size == 0) return true;
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = search_get_element(vec, i);
        if (!predicate(element, user_data)) {
            return false;
        }
    }
    
    return true;
}

size_t samrena_vector_count(const SamrenaVector* vec, const void* target,
                           SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return 0;
    }
    
    size_t count = 0;
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = search_get_element(vec, i);
        if (compare_fn(element, target, user_data) == 0) {
            count++;
        }
    }
    
    return count;
}

size_t samrena_vector_count_if(const SamrenaVector* vec, SamrenaPredicateFn predicate,
                              void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !predicate) {
        return 0;
    }
    
    size_t count = 0;
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = search_get_element(vec, i);
        if (predicate(element, user_data)) {
            count++;
        }
    }
    
    return count;
}
```

### 4. Range-Based Search
```c
size_t samrena_vector_find_range(const SamrenaVector* vec, size_t start, size_t end,
                                const void* target, SamrenaCompareFn compare_fn,
                                void* user_data) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return SAMRENA_NOT_FOUND;
    }
    
    // Validate and clamp range
    if (start >= vec->size) return SAMRENA_NOT_FOUND;
    if (end > vec->size) end = vec->size;
    if (start >= end) return SAMRENA_NOT_FOUND;
    
    for (size_t i = start; i < end; i++) {
        const void* element = search_get_element(vec, i);
        if (compare_fn(element, target, user_data) == 0) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

size_t samrena_vector_find_if_range(const SamrenaVector* vec, size_t start, size_t end,
                                   SamrenaPredicateFn predicate, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !predicate) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (start >= vec->size) return SAMRENA_NOT_FOUND;
    if (end > vec->size) end = vec->size;
    if (start >= end) return SAMRENA_NOT_FOUND;
    
    for (size_t i = start; i < end; i++) {
        const void* element = search_get_element(vec, i);
        if (predicate(element, user_data)) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}
```

### 5. Built-in Comparators
```c
// Standard comparison functions
int samrena_compare_int(const void* a, const void* b, void* user_data) {
    (void)user_data; // Unused
    int val_a = *(const int*)a;
    int val_b = *(const int*)b;
    return (val_a > val_b) - (val_a < val_b);
}

int samrena_compare_float(const void* a, const void* b, void* user_data) {
    (void)user_data;
    float val_a = *(const float*)a;
    float val_b = *(const float*)b;
    
    // Handle NaN and special cases
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    if (val_a == val_b) return 0;
    
    // Handle NaN cases (NaN is not equal to anything, including itself)
    if (val_a != val_a && val_b != val_b) return 0;  // Both NaN
    if (val_a != val_a) return 1;   // a is NaN, b is not
    return -1;                      // b is NaN, a is not
}

int samrena_compare_double(const void* a, const void* b, void* user_data) {
    (void)user_data;
    double val_a = *(const double*)a;
    double val_b = *(const double*)b;
    
    if (val_a < val_b) return -1;
    if (val_a > val_b) return 1;
    if (val_a == val_b) return 0;
    
    if (val_a != val_a && val_b != val_b) return 0;
    if (val_a != val_a) return 1;
    return -1;
}

int samrena_compare_string(const void* a, const void* b, void* user_data) {
    (void)user_data;
    const char* str_a = *(const char**)a;
    const char* str_b = *(const char**)b;
    
    if (!str_a && !str_b) return 0;
    if (!str_a) return -1;
    if (!str_b) return 1;
    
    return strcmp(str_a, str_b);
}

int samrena_compare_ptr(const void* a, const void* b, void* user_data) {
    (void)user_data;
    const void* ptr_a = *(const void**)a;
    const void* ptr_b = *(const void**)b;
    
    if (ptr_a < ptr_b) return -1;
    if (ptr_a > ptr_b) return 1;
    return 0;
}

int samrena_compare_bytes(const void* a, const void* b, void* user_data) {
    size_t* size_ptr = (size_t*)user_data;
    size_t size = size_ptr ? *size_ptr : sizeof(void*);
    return memcmp(a, b, size);
}
```

### 6. Type-Specific Optimized Search
```c
size_t samrena_vector_find_int(const SamrenaVector* vec, int target) {
    if (validate_vector(vec) != SAMRENA_SUCCESS) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->element_size != sizeof(int)) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_OPERATION, 
                         "Vector element size doesn't match int", vec);
        return SAMRENA_NOT_FOUND;
    }
    
    const int* data = (const int*)vec->data;
    for (size_t i = 0; i < vec->size; i++) {
        if (data[i] == target) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

size_t samrena_vector_find_float(const SamrenaVector* vec, float target) {
    if (validate_vector(vec) != SAMRENA_SUCCESS) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->element_size != sizeof(float)) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_OPERATION, 
                         "Vector element size doesn't match float", vec);
        return SAMRENA_NOT_FOUND;
    }
    
    const float* data = (const float*)vec->data;
    for (size_t i = 0; i < vec->size; i++) {
        // Use epsilon comparison for floating point
        if (fabsf(data[i] - target) < FLT_EPSILON) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

size_t samrena_vector_find_string(const SamrenaVector* vec, const char* target) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->element_size != sizeof(char*)) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_OPERATION, 
                         "Vector element size doesn't match string pointer", vec);
        return SAMRENA_NOT_FOUND;
    }
    
    const char** data = (const char**)vec->data;
    for (size_t i = 0; i < vec->size; i++) {
        if (data[i] && strcmp(data[i], target) == 0) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

size_t samrena_vector_find_bytes(const SamrenaVector* vec, const void* target) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS) {
        return SAMRENA_NOT_FOUND;
    }
    
    const char* data = (const char*)vec->data;
    const char* target_bytes = (const char*)target;
    
    for (size_t i = 0; i < vec->size; i++) {
        if (memcmp(data + (i * vec->element_size), target_bytes, vec->element_size) == 0) {
            return i;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}
```

### 7. Binary Search Implementation
```c
size_t samrena_vector_binary_search(const SamrenaVector* vec, const void* target,
                                   SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return SAMRENA_NOT_FOUND;
    }
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        const void* mid_element = search_get_element(vec, mid);
        int cmp = compare_fn(mid_element, target, user_data);
        
        if (cmp == 0) {
            return mid;
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

SamrenaBinarySearchResult samrena_vector_binary_search_insert_point(
    const SamrenaVector* vec, const void* target,
    SamrenaCompareFn compare_fn, void* user_data) {
    
    SamrenaBinarySearchResult result = {false, 0};
    
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return result;
    }
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        const void* mid_element = search_get_element(vec, mid);
        int cmp = compare_fn(mid_element, target, user_data);
        
        if (cmp == 0) {
            result.found = true;
            result.index = mid;
            return result;
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    result.found = false;
    result.index = left;  // Insertion point
    return result;
}

size_t samrena_vector_lower_bound(const SamrenaVector* vec, const void* target,
                                 SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return vec ? vec->size : 0;
    }
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        const void* mid_element = search_get_element(vec, mid);
        
        if (compare_fn(mid_element, target, user_data) < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return left;
}

size_t samrena_vector_upper_bound(const SamrenaVector* vec, const void* target,
                                 SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_search_params(vec, target) != SAMRENA_SUCCESS || !compare_fn) {
        return vec ? vec->size : 0;
    }
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        const void* mid_element = search_get_element(vec, mid);
        
        if (compare_fn(mid_element, target, user_data) <= 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return left;
}
```

### 8. QSort Comparator Wrapper
```c
int samrena_wrap_qsort_compare(const void* a, const void* b, void* user_data) {
    SamrenaQSortWrapper* wrapper = (SamrenaQSortWrapper*)user_data;
    if (!wrapper || !wrapper->qsort_fn) {
        return 0;
    }
    return wrapper->qsort_fn(a, b);
}
```

## Performance Optimizations

### SIMD-Optimized Search (Future Enhancement)
```c
#ifdef SAMRENA_USE_SIMD
// Vectorized search for int arrays
size_t samrena_vector_find_int_simd(const SamrenaVector* vec, int target) {
    if (vec->size < 8) {
        return samrena_vector_find_int(vec, target);  // Fallback for small arrays
    }
    
    // SIMD implementation for large arrays
    // Implementation would depend on target architecture (SSE, AVX, NEON, etc.)
    return samrena_vector_find_int(vec, target);  // Fallback for now
}
#endif
```

## Testing Strategy

### Unit Tests
```c
void test_linear_search() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);
    
    int values[] = {1, 3, 5, 7, 9, 11, 13};
    samrena_vector_push_array(vec, values, 7);
    
    // Test found
    assert(samrena_vector_find_int(vec, 7) == 3);
    
    // Test not found
    assert(samrena_vector_find_int(vec, 8) == SAMRENA_NOT_FOUND);
    
    // Test edge cases
    assert(samrena_vector_find_int(vec, 1) == 0);  // First element
    assert(samrena_vector_find_int(vec, 13) == 6); // Last element
    
    samrena_vector_destroy(vec);
}

void test_binary_search() {
    SamrenaVector* vec = create_sorted_int_vector();
    
    size_t index = samrena_vector_binary_search(vec, &target, samrena_compare_int, NULL);
    assert(index != SAMRENA_NOT_FOUND);
    
    SamrenaBinarySearchResult result = samrena_vector_binary_search_insert_point(
        vec, &new_value, samrena_compare_int, NULL);
    
    if (!result.found) {
        samrena_vector_insert(vec, result.index, &new_value);
        // Vector should still be sorted
    }
}
```

### Performance Benchmarks
- Linear vs binary search performance
- Type-specific vs generic search comparison
- Large dataset search performance
- Memory access pattern analysis

## Integration Notes
- Uses vector validation from error handling system
- Compatible with both owned and shared arena vectors
- Builds upon element access infrastructure
- Prepares foundation for future SIMD optimizations
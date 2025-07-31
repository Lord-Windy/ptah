# Task 19: Implement Binary Search

## Objective
Implement comprehensive binary search algorithms for sorted Samvector data, including standard binary search, lower/upper bounds, insertion point finding, and range queries with performance optimizations and type-safe variants.

## Dependencies
- Task 18: Implement Sorting (for sorted vector operations)
- Task 12: Implement Search Functions (for linear search fallback)
- Task 08: Implement Error Handling (for error reporting)
- Task 17: Implement Type Safety System (for type-safe binary search)

## Implementation Plan

### 1. Core Binary Search Infrastructure
```c
// samrena_vector_binary_search.c - Binary search implementation

#include "samrena_vector.h"
#include <assert.h>

// Binary search result with detailed information
typedef struct {
    bool found;           // True if exact match was found
    size_t index;         // Index of found element or insertion point
    size_t comparisons;   // Number of comparisons performed (for profiling)
} SamrenaBinarySearchResult;

// Enhanced result for range queries
typedef struct {
    bool found;           // True if any matching elements exist
    size_t first_index;   // Index of first matching element
    size_t last_index;    // Index of last matching element
    size_t count;         // Number of matching elements
} SamrenaBinarySearchRange;

// Internal comparison context
typedef struct {
    SamrenaCompareFn compare_fn;
    void* user_data;
    size_t element_size;
    size_t comparison_count;  // Track comparisons for analysis
} SamrenaBinarySearchContext;

// Get element pointer for binary search
static inline const void* bsearch_get_element(const SamrenaVector* vec, size_t index) {
    return (const char*)vec->data + (index * vec->element_size);
}

// Compare target with element at index
static inline int bsearch_compare_at_index(const SamrenaBinarySearchContext* ctx,
                                          const SamrenaVector* vec,
                                          const void* target, size_t index) {
    const void* element = bsearch_get_element(vec, index);
    ((SamrenaBinarySearchContext*)ctx)->comparison_count++;
    return ctx->compare_fn(target, element, ctx->user_data);
}
```

### 2. Standard Binary Search
```c
// Classic binary search - returns exact match or SIZE_MAX
size_t samrena_vector_binary_search(const SamrenaVector* vec, const void* target,
                                   SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->size == 0) return SAMRENA_NOT_FOUND;
    
    SamrenaBinarySearchContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size,
        .comparison_count = 0
    };
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = bsearch_compare_at_index(&context, vec, target, mid);
        
        if (cmp == 0) {
            return mid;  // Exact match found
        } else if (cmp < 0) {
            left = mid + 1;  // Target is in right half
        } else {
            right = mid;     // Target is in left half
        }
    }
    
    return SAMRENA_NOT_FOUND;  // Not found
}

// Binary search with detailed result information
SamrenaBinarySearchResult samrena_vector_binary_search_detailed(
    const SamrenaVector* vec, const void* target,
    SamrenaCompareFn compare_fn, void* user_data) {
    
    SamrenaBinarySearchResult result = {false, 0, 0};
    
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn) {
        return result;
    }
    
    if (vec->size == 0) {
        return result;
    }
    
    SamrenaBinarySearchContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size,
        .comparison_count = 0
    };
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = bsearch_compare_at_index(&context, vec, target, mid);
        
        if (cmp == 0) {
            result.found = true;
            result.index = mid;
            result.comparisons = context.comparison_count;
            return result;
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    // Not found - return insertion point
    result.found = false;
    result.index = left;
    result.comparisons = context.comparison_count;
    return result;
}
```

### 3. Lower and Upper Bound Search
```c
// Find first position where element could be inserted (lower bound)
size_t samrena_vector_lower_bound(const SamrenaVector* vec, const void* target,
                                 SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn) {
        return 0;
    }
    
    SamrenaBinarySearchContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size,
        .comparison_count = 0
    };
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = bsearch_compare_at_index(&context, vec, target, mid);
        
        if (cmp <= 0) {  // target <= element[mid]
            right = mid;
        } else {         // target > element[mid]
            left = mid + 1;
        }
    }
    
    return left;
}

// Find last position where element could be inserted (upper bound)
size_t samrena_vector_upper_bound(const SamrenaVector* vec, const void* target,
                                 SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn) {
        return 0;
    }
    
    SamrenaBinarySearchContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size,
        .comparison_count = 0
    };
    
    size_t left = 0;
    size_t right = vec->size;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = bsearch_compare_at_index(&context, vec, target, mid);
        
        if (cmp < 0) {   // target < element[mid]
            right = mid;
        } else {         // target >= element[mid]
            left = mid + 1;
        }
    }
    
    return left;
}

// Find range of all matching elements
SamrenaBinarySearchRange samrena_vector_equal_range(const SamrenaVector* vec, 
                                                   const void* target,
                                                   SamrenaCompareFn compare_fn, 
                                                   void* user_data) {
    SamrenaBinarySearchRange range = {false, 0, 0, 0};
    
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn) {
        return range;
    }
    
    if (vec->size == 0) {
        return range;
    }
    
    // Find lower and upper bounds
    size_t lower = samrena_vector_lower_bound(vec, target, compare_fn, user_data);
    size_t upper = samrena_vector_upper_bound(vec, target, compare_fn, user_data);
    
    if (lower < vec->size && lower < upper) {
        // Verify that we actually found matching elements
        SamrenaBinarySearchContext context = {
            .compare_fn = compare_fn,
            .user_data = user_data,
            .element_size = vec->element_size,
            .comparison_count = 0
        };
        
        if (bsearch_compare_at_index(&context, vec, target, lower) == 0) {
            range.found = true;
            range.first_index = lower;
            range.last_index = upper - 1;
            range.count = upper - lower;
        }
    }
    
    return range;
}
```

### 4. Insertion Point Search
```c
// Find insertion point to maintain sorted order
size_t samrena_vector_insertion_point(const SamrenaVector* vec, const void* target,
                                     SamrenaCompareFn compare_fn, void* user_data) {
    // Lower bound gives us the insertion point
    return samrena_vector_lower_bound(vec, target, compare_fn, user_data);
}

// Insert element at correct position to maintain sorted order
int samrena_vector_insert_sorted(SamrenaVector* vec, const void* element,
                                SamrenaCompareFn compare_fn, void* user_data) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!element || !compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, 
                         "Element or compare function is NULL", element);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    // Find insertion point
    size_t insertion_point = samrena_vector_insertion_point(vec, element, 
                                                           compare_fn, user_data);
    
    // Insert at the found position
    return samrena_vector_insert(vec, insertion_point, element);
}

// Insert element only if it doesn't already exist
int samrena_vector_insert_unique(SamrenaVector* vec, const void* element,
                                SamrenaCompareFn compare_fn, void* user_data) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!element || !compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, 
                         "Element or compare function is NULL", element);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    // Check if element already exists
    if (samrena_vector_binary_search(vec, element, compare_fn, user_data) != SAMRENA_NOT_FOUND) {
        // Element already exists - don't insert
        return SAMRENA_SUCCESS;
    }
    
    // Insert the new element
    return samrena_vector_insert_sorted(vec, element, compare_fn, user_data);
}
```

### 5. Range and Multi-Value Operations
```c
// Count elements in a specific range [min_val, max_val]
size_t samrena_vector_count_range(const SamrenaVector* vec, 
                                 const void* min_val, const void* max_val,
                                 SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !min_val || !max_val || !compare_fn) {
        return 0;
    }
    
    // Find bounds for both values
    size_t lower = samrena_vector_lower_bound(vec, min_val, compare_fn, user_data);
    size_t upper = samrena_vector_upper_bound(vec, max_val, compare_fn, user_data);
    
    return (upper > lower) ? (upper - lower) : 0;
}

// Find all elements in range [min_val, max_val]
size_t samrena_vector_find_range_elements(const SamrenaVector* vec,
                                         const void* min_val, const void* max_val,
                                         SamrenaCompareFn compare_fn, void* user_data,
                                         size_t* indices, size_t max_indices) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !min_val || !max_val || 
        !compare_fn || (!indices && max_indices > 0)) {
        return 0;
    }
    
    size_t lower = samrena_vector_lower_bound(vec, min_val, compare_fn, user_data);
    size_t upper = samrena_vector_upper_bound(vec, max_val, compare_fn, user_data);
    
    size_t count = 0;
    for (size_t i = lower; i < upper && count < max_indices; i++) {
        indices[count] = i;
        count++;
    }
    
    return count;
}

// Remove all elements equal to target value
size_t samrena_vector_remove_equal(SamrenaVector* vec, const void* target,
                                  SamrenaCompareFn compare_fn, void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn) {
        return 0;
    }
    
    SamrenaBinarySearchRange range = samrena_vector_equal_range(vec, target, 
                                                               compare_fn, user_data);
    
    if (!range.found) {
        return 0;  // No elements to remove
    }
    
    // Remove the range of matching elements
    int result = samrena_vector_remove_range(vec, range.first_index, range.count);
    
    return (result == SAMRENA_SUCCESS) ? range.count : 0;
}
```

### 6. Type-Safe Binary Search Extensions
```c
// Type-safe binary search functions
#define SAMRENA_VECTOR_DEFINE_BINARY_SEARCH_FUNCTIONS(T) \
    static inline size_t samrena_vector_##T##_binary_search( \
        const SamrenaVector_##T* vec, T target, \
        int (*compare)(const T*, const T*)) { \
        \
        if (samrena_vector_##T##_check_valid(vec) != SAMRENA_SUCCESS) { \
            return SAMRENA_NOT_FOUND; \
        } \
        \
        if (compare) { \
            struct compare_wrapper { int (*typed_compare)(const T*, const T*); }; \
            struct compare_wrapper wrapper = {compare}; \
            \
            int generic_compare(const void* a, const void* b, void* user_data) { \
                struct compare_wrapper* w = (struct compare_wrapper*)user_data; \
                return w->typed_compare((const T*)a, (const T*)b); \
            } \
            \
            return samrena_vector_binary_search(vec->_internal, &target, \
                                               generic_compare, &wrapper); \
        } else { \
            return samrena_vector_find_bytes(vec->_internal, &target); \
        } \
    } \
    \
    static inline size_t samrena_vector_##T##_lower_bound( \
        const SamrenaVector_##T* vec, T target, \
        int (*compare)(const T*, const T*)) { \
        \
        if (samrena_vector_##T##_check_valid(vec) != SAMRENA_SUCCESS || !compare) { \
            return 0; \
        } \
        \
        struct compare_wrapper { int (*typed_compare)(const T*, const T*); }; \
        struct compare_wrapper wrapper = {compare}; \
        \
        int generic_compare(const void* a, const void* b, void* user_data) { \
            struct compare_wrapper* w = (struct compare_wrapper*)user_data; \
            return w->typed_compare((const T*)a, (const T*)b); \
        } \
        \
        return samrena_vector_lower_bound(vec->_internal, &target, \
                                         generic_compare, &wrapper); \
    } \
    \
    static inline size_t samrena_vector_##T##_upper_bound( \
        const SamrenaVector_##T* vec, T target, \
        int (*compare)(const T*, const T*)) { \
        \
        if (samrena_vector_##T##_check_valid(vec) != SAMRENA_SUCCESS || !compare) { \
            return 0; \
        } \
        \
        struct compare_wrapper { int (*typed_compare)(const T*, const T*); }; \
        struct compare_wrapper wrapper = {compare}; \
        \
        int generic_compare(const void* a, const void* b, void* user_data) { \
            struct compare_wrapper* w = (struct compare_wrapper*)user_data; \
            return w->typed_compare((const T*)a, (const T*)b); \
        } \
        \
        return samrena_vector_upper_bound(vec->_internal, &target, \
                                         generic_compare, &wrapper); \
    } \
    \
    static inline int samrena_vector_##T##_insert_sorted( \
        SamrenaVector_##T* vec, T element, \
        int (*compare)(const T*, const T*)) { \
        \
        int err = samrena_vector_##T##_check_valid(vec); \
        if (err != SAMRENA_SUCCESS) return err; \
        \
        if (!compare) return SAMRENA_ERROR_NULL_POINTER; \
        \
        struct compare_wrapper { int (*typed_compare)(const T*, const T*); }; \
        struct compare_wrapper wrapper = {compare}; \
        \
        int generic_compare(const void* a, const void* b, void* user_data) { \
            struct compare_wrapper* w = (struct compare_wrapper*)user_data; \
            return w->typed_compare((const T*)a, (const T*)b); \
        } \
        \
        return samrena_vector_insert_sorted(vec->_internal, &element, \
                                           generic_compare, &wrapper); \
    }

// Add binary search functions to common types
SAMRENA_VECTOR_DEFINE_BINARY_SEARCH_FUNCTIONS(int)
SAMRENA_VECTOR_DEFINE_BINARY_SEARCH_FUNCTIONS(float)
SAMRENA_VECTOR_DEFINE_BINARY_SEARCH_FUNCTIONS(double)
```

### 7. Performance Optimizations
```c
// Optimized binary search for specific types
static inline size_t binary_search_int_optimized(const int* data, size_t count, int target) {
    size_t left = 0;
    size_t right = count;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        
        if (data[mid] == target) {
            return mid;
        } else if (data[mid] < target) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return SAMRENA_NOT_FOUND;
}

// Use optimized version for int vectors when possible
size_t samrena_vector_binary_search_int_fast(const SamrenaVector* vec, int target) {
    if (validate_vector(vec) != SAMRENA_SUCCESS) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->element_size != sizeof(int)) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_OPERATION, 
                         "Vector element size doesn't match int", vec);
        return SAMRENA_NOT_FOUND;
    }
    
    return binary_search_int_optimized((const int*)vec->data, vec->size, target);
}

// Interpolation search for uniformly distributed numeric data
size_t samrena_vector_interpolation_search(const SamrenaVector* vec, const void* target,
                                          SamrenaCompareFn compare_fn, void* user_data,
                                          double (*to_double)(const void*)) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !target || !compare_fn || !to_double) {
        return SAMRENA_NOT_FOUND;
    }
    
    if (vec->size == 0) return SAMRENA_NOT_FOUND;
    
    SamrenaBinarySearchContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size,
        .comparison_count = 0
    };
    
    size_t left = 0;
    size_t right = vec->size - 1;
    double target_val = to_double(target);
    
    // Fallback to binary search if interpolation isn't beneficial
    while (left <= right && left < vec->size && right < vec->size) {
        // Get values at boundaries
        const void* left_elem = bsearch_get_element(vec, left);
        const void* right_elem = bsearch_get_element(vec, right);
        double left_val = to_double(left_elem);
        double right_val = to_double(right_elem);
        
        // Check if target is in range
        int left_cmp = bsearch_compare_at_index(&context, vec, target, left);
        if (left_cmp == 0) return left;
        if (left_cmp > 0) return SAMRENA_NOT_FOUND;  // Target too small
        
        int right_cmp = bsearch_compare_at_index(&context, vec, target, right);
        if (right_cmp == 0) return right;
        if (right_cmp < 0) return SAMRENA_NOT_FOUND;  // Target too large
        
        // Interpolate position
        if (right_val == left_val) {
            // Avoid division by zero - fall back to binary search
            size_t mid = left + (right - left) / 2;
            int mid_cmp = bsearch_compare_at_index(&context, vec, target, mid);
            
            if (mid_cmp == 0) return mid;
            else if (mid_cmp < 0) left = mid + 1;
            else right = mid - 1;
        } else {
            // Interpolation formula
            size_t pos = left + (size_t)((target_val - left_val) / 
                                        (right_val - left_val) * (right - left));
            
            // Clamp to valid range
            if (pos < left) pos = left;
            if (pos > right) pos = right;
            
            int pos_cmp = bsearch_compare_at_index(&context, vec, target, pos);
            
            if (pos_cmp == 0) {
                return pos;
            } else if (pos_cmp < 0) {
                left = pos + 1;
            } else {
                right = pos - 1;
            }
        }
        
        // Prevent infinite loops
        if (right <= left) break;
    }
    
    return SAMRENA_NOT_FOUND;
}
```

### 8. Verification and Debugging
```c
// Verify that vector is properly sorted
bool samrena_vector_is_sorted(const SamrenaVector* vec, SamrenaCompareFn compare_fn,
                             void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !compare_fn) {
        return false;
    }
    
    if (vec->size <= 1) return true;
    
    SamrenaBinarySearchContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size,
        .comparison_count = 0
    };
    
    for (size_t i = 1; i < vec->size; i++) {
        const void* prev = bsearch_get_element(vec, i - 1);
        const void* curr = bsearch_get_element(vec, i);
        
        if (compare_fn(prev, curr, user_data) > 0) {
            return false;  // Not sorted
        }
    }
    
    return true;
}

// Find first position where sorting invariant is violated
size_t samrena_vector_find_unsorted_position(const SamrenaVector* vec, 
                                            SamrenaCompareFn compare_fn,
                                            void* user_data) {
    if (validate_vector(vec) != SAMRENA_SUCCESS || !compare_fn) {
        return SAMRENA_NOT_FOUND;
    }
    
    for (size_t i = 1; i < vec->size; i++) {
        const void* prev = bsearch_get_element(vec, i - 1);
        const void* curr = bsearch_get_element(vec, i);
        
        if (compare_fn(prev, curr, user_data) > 0) {
            return i;  // Found unsorted position
        }
    }
    
    return SAMRENA_NOT_FOUND;  // Vector is sorted
}
```

## Testing Strategy

### Correctness Tests
```c
void test_binary_search_basic() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);
    
    // Create sorted data
    int values[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    samrena_vector_push_array(vec, values, 10);
    
    // Test exact matches
    for (int i = 0; i < 10; i++) {
        size_t index = samrena_vector_binary_search(vec, &values[i], 
                                                   samrena_compare_int, NULL);
        assert(index == i);
    }
    
    // Test non-existent values
    int not_found = 8;
    assert(samrena_vector_binary_search(vec, &not_found, 
                                       samrena_compare_int, NULL) == SAMRENA_NOT_FOUND);
    
    samrena_vector_destroy(vec);
}

void test_bounds_and_ranges() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 15);
    
    // Data with duplicates: [1, 3, 3, 3, 5, 7, 7, 9, 11, 11, 11, 11, 13]
    int values[] = {1, 3, 3, 3, 5, 7, 7, 9, 11, 11, 11, 11, 13};
    samrena_vector_push_array(vec, values, 13);
    
    // Test lower/upper bounds for value 3
    int target = 3;
    size_t lower = samrena_vector_lower_bound(vec, &target, samrena_compare_int, NULL);
    size_t upper = samrena_vector_upper_bound(vec, &target, samrena_compare_int, NULL);
    
    assert(lower == 1);  // First occurrence of 3
    assert(upper == 4);  // One past last occurrence of 3
    
    // Test equal range
    SamrenaBinarySearchRange range = samrena_vector_equal_range(vec, &target, 
                                                               samrena_compare_int, NULL);
    assert(range.found == true);
    assert(range.first_index == 1);
    assert(range.last_index == 3);
    assert(range.count == 3);
    
    samrena_vector_destroy(vec);
}

void test_sorted_insertion() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);
    
    // Insert values in random order
    int values[] = {5, 2, 8, 1, 9, 3, 7};
    for (size_t i = 0; i < 7; i++) {
        assert(samrena_vector_insert_sorted(vec, &values[i], 
                                           samrena_compare_int, NULL) == SAMRENA_SUCCESS);
    }
    
    // Verify vector is sorted
    assert(samrena_vector_is_sorted(vec, samrena_compare_int, NULL));
    
    // Verify all values are present
    for (size_t i = 0; i < 7; i++) {
        assert(samrena_vector_binary_search(vec, &values[i], 
                                           samrena_compare_int, NULL) != SAMRENA_NOT_FOUND);
    }
    
    samrena_vector_destroy(vec);
}
```

### Performance Benchmarks
```c
void benchmark_search_algorithms() {
    const size_t sizes[] = {1000, 10000, 100000, 1000000};
    const size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (size_t i = 0; i < num_sizes; i++) {
        printf("Testing size %zu:\n", sizes[i]);
        
        // Create sorted vector
        SamrenaVector* vec = create_sorted_int_vector(sizes[i]);
        int target = sizes[i] / 2;  // Middle value
        
        // Benchmark linear search
        clock_t start = clock();
        for (int j = 0; j < 1000; j++) {
            samrena_vector_find_int(vec, target);
        }
        clock_t linear_time = clock() - start;
        
        // Benchmark binary search
        start = clock();
        for (int j = 0; j < 1000; j++) {
            samrena_vector_binary_search_int_fast(vec, target);
        }
        clock_t binary_time = clock() - start;
        
        printf("  Linear: %ld ms, Binary: %ld ms, Speedup: %.2fx\n",
               linear_time, binary_time, (double)linear_time / binary_time);
        
        samrena_vector_destroy(vec);
    }
}
```

## Integration Notes
- Requires vectors to be sorted before use
- Compatible with existing comparison functions
- Works with both owned and shared arena vectors
- Integrates with type-safe vector system
- Provides both simple and detailed result interfaces
- Optimized for different data types and patterns
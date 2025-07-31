# Task 18: Implement Sorting

## Objective
Implement comprehensive sorting algorithms for Samvector, including quicksort, mergesort, and heapsort with custom comparators, stable sorting options, and performance optimizations for different data types and sizes.

## Dependencies
- Task 15: Implement Utility Functions (for swap operations)
- Task 12: Implement Search Functions (for comparison functions)
- Task 08: Implement Error Handling (for error reporting)
- Task 17: Implement Type Safety System (for type-safe sorting)

## Implementation Plan

### 1. Core Sorting Infrastructure
```c
// samrena_vector_sort.c - Core sorting implementation

#include "samrena_vector.h"
#include <string.h>
#include <stdlib.h>

// Sorting algorithm selection threshold
#define SAMRENA_QUICKSORT_THRESHOLD 16    // Use insertion sort below this
#define SAMRENA_MERGESORT_THRESHOLD 32    // Use quicksort below this
#define SAMRENA_INTROSORT_DEPTH_LIMIT 64  // Max recursion depth

// Internal comparison function wrapper
typedef struct {
    SamrenaCompareFn compare_fn;
    void* user_data;
    size_t element_size;
} SamrenaSortContext;

// Fast element swap for sorting
static inline void sort_swap_elements(void* data, size_t element_size, 
                                     size_t index1, size_t index2) {
    if (index1 == index2) return;
    
    char* ptr1 = (char*)data + (index1 * element_size);
    char* ptr2 = (char*)data + (index2 * element_size);
    
    if (element_size <= 16) {
        // Use stack buffer for small elements
        char temp[16];
        memcpy(temp, ptr1, element_size);
        memcpy(ptr1, ptr2, element_size);
        memcpy(ptr2, temp, element_size);
    } else {
        // Use dynamic allocation for large elements
        char* temp = malloc(element_size);
        if (temp) {
            memcpy(temp, ptr1, element_size);
            memcpy(ptr1, ptr2, element_size);
            memcpy(ptr2, temp, element_size);
            free(temp);
        }
    }
}

// Get element pointer by index
static inline void* sort_get_element(void* data, size_t element_size, size_t index) {
    return (char*)data + (index * element_size);
}

// Compare two elements using context
static inline int compare_elements(const SamrenaSortContext* ctx, 
                                  void* data, size_t index1, size_t index2) {
    void* elem1 = sort_get_element(data, ctx->element_size, index1);
    void* elem2 = sort_get_element(data, ctx->element_size, index2);
    return ctx->compare_fn(elem1, elem2, ctx->user_data);
}
```

### 2. Insertion Sort (for small arrays)
```c
static void insertion_sort_range(void* data, size_t element_size, 
                                size_t start, size_t end,
                                const SamrenaSortContext* ctx) {
    for (size_t i = start + 1; i < end; i++) {
        size_t j = i;
        
        // Find insertion point by shifting elements
        while (j > start && 
               compare_elements(ctx, data, j, j - 1) < 0) {
            sort_swap_elements(data, element_size, j, j - 1);
            j--;
        }
    }
}

static void insertion_sort(void* data, size_t count, size_t element_size,
                          const SamrenaSortContext* ctx) {
    if (count <= 1) return;
    insertion_sort_range(data, element_size, 0, count, ctx);
}
```

### 3. Quicksort Implementation
```c
// Partition function for quicksort (Hoare partition scheme)
static size_t quicksort_partition(void* data, size_t element_size,
                                 size_t start, size_t end,
                                 const SamrenaSortContext* ctx) {
    // Choose median-of-three as pivot
    size_t mid = start + (end - start) / 2;
    
    // Sort start, mid, end to get median
    if (compare_elements(ctx, data, start, mid) > 0) {
        sort_swap_elements(data, element_size, start, mid);
    }
    if (compare_elements(ctx, data, mid, end - 1) > 0) {
        sort_swap_elements(data, element_size, mid, end - 1);
    }
    if (compare_elements(ctx, data, start, mid) > 0) {
        sort_swap_elements(data, element_size, start, mid);
    }
    
    // Move pivot to start position
    sort_swap_elements(data, element_size, start, mid);
    
    size_t pivot_index = start;
    size_t left = start + 1;
    size_t right = end - 1;
    
    while (true) {
        // Find element on left that should be on right
        while (left <= right && 
               compare_elements(ctx, data, left, pivot_index) <= 0) {
            left++;
        }
        
        // Find element on right that should be on left
        while (left <= right && 
               compare_elements(ctx, data, right, pivot_index) > 0) {
            right--;
        }
        
        if (left > right) break;
        
        sort_swap_elements(data, element_size, left, right);
        left++;
        right--;
    }
    
    // Place pivot in final position
    sort_swap_elements(data, element_size, pivot_index, right);
    return right;
}

static void quicksort_recursive(void* data, size_t element_size,
                               size_t start, size_t end,
                               const SamrenaSortContext* ctx) {
    if (end - start <= 1) return;
    
    // Use insertion sort for small subarrays
    if (end - start <= SAMRENA_QUICKSORT_THRESHOLD) {
        insertion_sort_range(data, element_size, start, end, ctx);
        return;
    }
    
    size_t pivot = quicksort_partition(data, element_size, start, end, ctx);
    
    // Recursively sort partitions
    if (pivot > start) {
        quicksort_recursive(data, element_size, start, pivot, ctx);
    }
    if (pivot + 1 < end) {
        quicksort_recursive(data, element_size, pivot + 1, end, ctx);
    }
}

static void quicksort(void* data, size_t count, size_t element_size,
                     const SamrenaSortContext* ctx) {
    if (count <= 1) return;
    quicksort_recursive(data, element_size, 0, count, ctx);
}
```

### 4. Mergesort Implementation (Stable)
```c
static void merge(void* data, size_t element_size,
                 size_t start, size_t mid, size_t end,
                 void* temp_buffer, const SamrenaSortContext* ctx) {
    size_t left = start;
    size_t right = mid;
    size_t temp_index = 0;
    
    // Merge the two sorted halves
    while (left < mid && right < end) {
        void* left_elem = sort_get_element(data, element_size, left);
        void* right_elem = sort_get_element(data, element_size, right);
        void* temp_elem = sort_get_element(temp_buffer, element_size, temp_index);
        
        if (compare_elements(ctx, data, left, right) <= 0) {
            memcpy(temp_elem, left_elem, element_size);
            left++;
        } else {
            memcpy(temp_elem, right_elem, element_size);
            right++;
        }
        temp_index++;
    }
    
    // Copy remaining elements
    while (left < mid) {
        void* left_elem = sort_get_element(data, element_size, left);
        void* temp_elem = sort_get_element(temp_buffer, element_size, temp_index);
        memcpy(temp_elem, left_elem, element_size);
        left++;
        temp_index++;
    }
    
    while (right < end) {
        void* right_elem = sort_get_element(data, element_size, right);
        void* temp_elem = sort_get_element(temp_buffer, element_size, temp_index);
        memcpy(temp_elem, right_elem, element_size);
        right++;
        temp_index++;
    }
    
    // Copy back to original array
    memcpy(sort_get_element(data, element_size, start), temp_buffer,
           (end - start) * element_size);
}

static void mergesort_recursive(void* data, size_t element_size,
                               size_t start, size_t end,
                               void* temp_buffer, const SamrenaSortContext* ctx) {
    if (end - start <= 1) return;
    
    // Use insertion sort for small subarrays
    if (end - start <= SAMRENA_QUICKSORT_THRESHOLD) {
        insertion_sort_range(data, element_size, start, end, ctx);
        return;
    }
    
    size_t mid = start + (end - start) / 2;
    
    // Recursively sort both halves
    mergesort_recursive(data, element_size, start, mid, temp_buffer, ctx);
    mergesort_recursive(data, element_size, mid, end, temp_buffer, ctx);
    
    // Merge the sorted halves
    merge(data, element_size, start, mid, end, temp_buffer, ctx);
}

static int mergesort(void* data, size_t count, size_t element_size,
                    const SamrenaSortContext* ctx) {
    if (count <= 1) return SAMRENA_SUCCESS;
    
    // Allocate temporary buffer
    void* temp_buffer = malloc(count * element_size);
    if (!temp_buffer) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_ALLOCATION_FAILED, 
                         "Failed to allocate merge buffer", NULL);
        return SAMRENA_ERROR_ALLOCATION_FAILED;
    }
    
    mergesort_recursive(data, element_size, 0, count, temp_buffer, ctx);
    
    free(temp_buffer);
    return SAMRENA_SUCCESS;
}
```

### 5. Heapsort Implementation
```c
// Heapify operation for max heap
static void heapify(void* data, size_t element_size, size_t count,
                   size_t root, const SamrenaSortContext* ctx) {
    size_t largest = root;
    size_t left = 2 * root + 1;
    size_t right = 2 * root + 2;
    
    // Find largest among root, left child, right child
    if (left < count && 
        compare_elements(ctx, data, left, largest) > 0) {
        largest = left;
    }
    
    if (right < count && 
        compare_elements(ctx, data, right, largest) > 0) {
        largest = right;
    }
    
    // If largest is not root, swap and continue heapifying
    if (largest != root) {
        sort_swap_elements(data, element_size, root, largest);
        heapify(data, element_size, count, largest, ctx);
    }
}

static void heapsort(void* data, size_t count, size_t element_size,
                    const SamrenaSortContext* ctx) {
    if (count <= 1) return;
    
    // Build max heap
    for (int i = count / 2 - 1; i >= 0; i--) {
        heapify(data, element_size, count, i, ctx);
    }
    
    // Extract elements from heap one by one
    for (size_t i = count - 1; i > 0; i--) {
        // Move current root to end
        sort_swap_elements(data, element_size, 0, i);
        
        // Heapify reduced heap
        heapify(data, element_size, i, 0, ctx);
    }
}
```

### 6. Introsort (Introspective Sort)
```c
static void introsort_recursive(void* data, size_t element_size,
                               size_t start, size_t end,
                               int depth_limit, const SamrenaSortContext* ctx) {
    if (end - start <= 1) return;
    
    // Use insertion sort for small subarrays
    if (end - start <= SAMRENA_QUICKSORT_THRESHOLD) {
        insertion_sort_range(data, element_size, start, end, ctx);
        return;
    }
    
    // Use heapsort if recursion depth is too high
    if (depth_limit == 0) {
        // Convert to heapsort for this subarray
        void* subarray = sort_get_element(data, element_size, start);
        heapsort(subarray, end - start, element_size, ctx);
        return;
    }
    
    // Otherwise use quicksort
    size_t pivot = quicksort_partition(data, element_size, start, end, ctx);
    
    introsort_recursive(data, element_size, start, pivot, depth_limit - 1, ctx);
    introsort_recursive(data, element_size, pivot + 1, end, depth_limit - 1, ctx);
}

static void introsort(void* data, size_t count, size_t element_size,
                     const SamrenaSortContext* ctx) {
    if (count <= 1) return;
    
    // Calculate depth limit (2 * log2(count))
    int depth_limit = 0;
    size_t n = count;
    while (n > 1) {
        n /= 2;
        depth_limit++;
    }
    depth_limit *= 2;
    
    introsort_recursive(data, element_size, 0, count, depth_limit, ctx);
}
```

### 7. Public Sorting API
```c
// Main sorting function with algorithm selection
int samrena_vector_sort(SamrenaVector* vec, SamrenaCompareFn compare_fn, 
                       void* user_data) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Compare function is NULL", compare_fn);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (vec->size <= 1) return SAMRENA_SUCCESS;
    
    SamrenaSortContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size
    };
    
    // Use introsort as default (best general-purpose algorithm)
    introsort(vec->data, vec->size, vec->element_size, &context);
    
    return SAMRENA_SUCCESS;
}

// Stable sort (guaranteed stable)
int samrena_vector_stable_sort(SamrenaVector* vec, SamrenaCompareFn compare_fn,
                              void* user_data) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Compare function is NULL", compare_fn);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (vec->size <= 1) return SAMRENA_SUCCESS;
    
    SamrenaSortContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size
    };
    
    return mergesort(vec->data, vec->size, vec->element_size, &context);
}

// Sort with specific algorithm
typedef enum {
    SAMRENA_SORT_QUICKSORT,
    SAMRENA_SORT_MERGESORT,
    SAMRENA_SORT_HEAPSORT,
    SAMRENA_SORT_INTROSORT,
    SAMRENA_SORT_INSERTION
} SamrenaSortAlgorithm;

int samrena_vector_sort_with_algorithm(SamrenaVector* vec, SamrenaCompareFn compare_fn,
                                      void* user_data, SamrenaSortAlgorithm algorithm) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Compare function is NULL", compare_fn);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (vec->size <= 1) return SAMRENA_SUCCESS;
    
    SamrenaSortContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size
    };
    
    switch (algorithm) {
        case SAMRENA_SORT_QUICKSORT:
            quicksort(vec->data, vec->size, vec->element_size, &context);
            return SAMRENA_SUCCESS;
            
        case SAMRENA_SORT_MERGESORT:
            return mergesort(vec->data, vec->size, vec->element_size, &context);
            
        case SAMRENA_SORT_HEAPSORT:
            heapsort(vec->data, vec->size, vec->element_size, &context);
            return SAMRENA_SUCCESS;
            
        case SAMRENA_SORT_INTROSORT:
            introsort(vec->data, vec->size, vec->element_size, &context);
            return SAMRENA_SUCCESS;
            
        case SAMRENA_SORT_INSERTION:
            insertion_sort(vec->data, vec->size, vec->element_size, &context);
            return SAMRENA_SUCCESS;
            
        default:
            SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_OPERATION, "Unknown sort algorithm", NULL);
            return SAMRENA_ERROR_INVALID_OPERATION;
    }
}
```

### 8. Partial and Range Sorting
```c
// Sort only a range of the vector
int samrena_vector_sort_range(SamrenaVector* vec, size_t start, size_t end,
                             SamrenaCompareFn compare_fn, void* user_data) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!compare_fn) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Compare function is NULL", compare_fn);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (start >= vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "Start index out of bounds", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    if (end > vec->size) end = vec->size;
    if (start >= end) return SAMRENA_SUCCESS;
    
    SamrenaSortContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size
    };
    
    introsort_recursive(vec->data, vec->element_size, start, end, 
                       SAMRENA_INTROSORT_DEPTH_LIMIT, &context);
    
    return SAMRENA_SUCCESS;
}

// Partial sort - sort first k elements
int samrena_vector_partial_sort(SamrenaVector* vec, size_t k,
                               SamrenaCompareFn compare_fn, void* user_data) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (k >= vec->size) {
        // Sort entire vector
        return samrena_vector_sort(vec, compare_fn, user_data);
    }
    
    if (k == 0) return SAMRENA_SUCCESS;
    
    SamrenaSortContext context = {
        .compare_fn = compare_fn,
        .user_data = user_data,
        .element_size = vec->element_size
    };
    
    // Use heap-based partial sort
    // Build max heap for first k elements
    for (int i = k / 2 - 1; i >= 0; i--) {
        heapify(vec->data, vec->element_size, k, i, &context);
    }
    
    // For each remaining element, if it's smaller than max, replace max
    for (size_t i = k; i < vec->size; i++) {
        if (compare_elements(&context, vec->data, i, 0) < 0) {
            sort_swap_elements(vec->data, vec->element_size, i, 0);
            heapify(vec->data, vec->element_size, k, 0, &context);
        }
    }
    
    // Sort the k elements
    heapsort(vec->data, k, vec->element_size, &context);
    
    return SAMRENA_SUCCESS;
}
```

### 9. Type-Safe Sorting Extensions
```c
// Type-safe sorting macro
#define SAMRENA_VECTOR_DEFINE_SORT_FUNCTIONS(T) \
    static inline int samrena_vector_##T##_sort(SamrenaVector_##T* vec, \
                                               int (*compare)(const T*, const T*)) { \
        if (samrena_vector_##T##_check_valid(vec) != SAMRENA_SUCCESS) { \
            return SAMRENA_ERROR_INVALID_STATE; \
        } \
        \
        if (!compare) { \
            /* Use byte-wise comparison as fallback */ \
            return samrena_vector_sort(vec->_internal, samrena_compare_bytes, \
                                     &vec->_internal->element_size); \
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
        return samrena_vector_sort(vec->_internal, generic_compare, &wrapper); \
    } \
    \
    static inline int samrena_vector_##T##_stable_sort(SamrenaVector_##T* vec, \
                                                      int (*compare)(const T*, const T*)) { \
        if (samrena_vector_##T##_check_valid(vec) != SAMRENA_SUCCESS) { \
            return SAMRENA_ERROR_INVALID_STATE; \
        } \
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
        return samrena_vector_stable_sort(vec->_internal, generic_compare, &wrapper); \
    }

// Add sorting functions to common types
SAMRENA_VECTOR_DEFINE_SORT_FUNCTIONS(int)
SAMRENA_VECTOR_DEFINE_SORT_FUNCTIONS(float)
SAMRENA_VECTOR_DEFINE_SORT_FUNCTIONS(double)
```

## Performance Optimizations

### Algorithm Selection Heuristics
```c
// Automatically choose best algorithm based on data characteristics
static SamrenaSortAlgorithm choose_optimal_algorithm(SamrenaVector* vec, 
                                                    SamrenaCompareFn compare_fn,
                                                    void* user_data) {
    // Small arrays: insertion sort
    if (vec->size < 16) return SAMRENA_SORT_INSERTION;
    
    // Large arrays: introsort (hybrid)
    if (vec->size > 1000) return SAMRENA_SORT_INTROSORT;
    
    // Check if data is already mostly sorted
    size_t inversions = 0;
    SamrenaSortContext context = {compare_fn, user_data, vec->element_size};
    
    for (size_t i = 1; i < vec->size && inversions < vec->size / 4; i++) {
        if (compare_elements(&context, vec->data, i - 1, i) > 0) {
            inversions++;
        }
    }
    
    // If mostly sorted, use insertion sort
    if (inversions < vec->size / 8) return SAMRENA_SORT_INSERTION;
    
    // Default to introsort
    return SAMRENA_SORT_INTROSORT;
}
```

### SIMD Optimizations (Future Enhancement)
```c
#ifdef SAMRENA_USE_SIMD
// SIMD-optimized sorting for specific types
static void simd_sort_int32(int32_t* data, size_t count) {
    // Implementation would use vectorized comparison and swapping
    // Fall back to standard algorithm for now
    // quicksort(data, count, sizeof(int32_t), &default_int_context);
}
#endif
```

## Testing Strategy

### Correctness Tests
```c
void test_sorting_correctness() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);
    
    // Test with random data
    int values[] = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    samrena_vector_push_array(vec, values, 9);
    
    assert(samrena_vector_sort(vec, samrena_compare_int, NULL) == SAMRENA_SUCCESS);
    
    // Verify sorted order
    for (size_t i = 1; i < vec->size; i++) {
        int* prev = (int*)samrena_vector_at(vec, i - 1);
        int* curr = (int*)samrena_vector_at(vec, i);
        assert(*prev <= *curr);
    }
    
    samrena_vector_destroy(vec);
}

void test_stable_sort() {
    typedef struct { int key; int value; } KeyValue;
    
    SAMRENA_VECTOR_DEFINE_TYPE_EXTENDED(KeyValue);
    
    SamrenaVector_KeyValue vec = samrena_vector_KeyValue_init_owned(5);
    
    KeyValue items[] = {{1, 10}, {2, 20}, {1, 30}, {2, 40}};
    samrena_vector_KeyValue_push_array(&vec, items, 4);
    
    int compare_by_key(const KeyValue* a, const KeyValue* b) {
        return a->key - b->key;
    }
    
    // Stable sort should preserve relative order of equal elements
    assert(samrena_vector_KeyValue_stable_sort(&vec, compare_by_key) == SAMRENA_SUCCESS);
    
    KeyValue* first = samrena_vector_KeyValue_at(&vec, 0);
    KeyValue* second = samrena_vector_KeyValue_at(&vec, 1);
    
    assert(first->key == 1 && first->value == 10);  // Original order preserved
    assert(second->key == 1 && second->value == 30);
    
    samrena_vector_KeyValue_destroy(&vec);
}
```

### Performance Benchmarks
```c
void benchmark_sorting_algorithms() {
    const size_t sizes[] = {100, 1000, 10000, 100000};
    const size_t num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (size_t i = 0; i < num_sizes; i++) {
        printf("Testing size %zu:\n", sizes[i]);
        
        // Create random data
        SamrenaVector* vec = create_random_int_vector(sizes[i]);
        
        // Test each algorithm
        SamrenaSortAlgorithm algorithms[] = {
            SAMRENA_SORT_QUICKSORT,
            SAMRENA_SORT_MERGESORT,
            SAMRENA_SORT_HEAPSORT,
            SAMRENA_SORT_INTROSORT
        };
        
        for (size_t j = 0; j < 4; j++) {
            SamrenaVector* copy = samrena_vector_copy(vec);
            
            clock_t start = clock();
            samrena_vector_sort_with_algorithm(copy, samrena_compare_int, 
                                             NULL, algorithms[j]);
            clock_t end = clock();
            
            printf("  Algorithm %d: %ld ms\n", (int)algorithms[j], 
                   (end - start) * 1000 / CLOCKS_PER_SEC);
            
            samrena_vector_destroy(copy);
        }
        
        samrena_vector_destroy(vec);
    }
}
```

## Integration Notes
- Compatible with existing vector API
- Works with both owned and shared arena vectors
- Supports custom comparison functions
- Integrates with type-safe vector system
- Provides multiple algorithms for different use cases
- Optimized for common data patterns
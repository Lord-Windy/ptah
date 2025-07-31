# Task 11: Design Search Interface

## Objective
Design a comprehensive search and query interface for Samvector that provides efficient element location, existence checking, and custom comparison capabilities.

## Requirements
- Linear search with custom comparators for unsorted vectors
- Efficient existence checking without element retrieval
- Support for both equality and custom comparison functions
- Type-safe search operations where possible
- Integration with standard C comparison function signatures
- Performance-optimized search paths for common data types

## Proposed API

### Basic Search Functions
```c
// Standard comparison function signature
typedef int (*SamrenaCompareFn)(const void* a, const void* b, void* user_data);

// Predicate function for boolean tests
typedef bool (*SamrenaPredicateFn)(const void* element, void* user_data);

// Find first element matching comparator (returns index or SIZE_MAX if not found)
size_t samrena_vector_find(const SamrenaVector* vec, const void* target, 
                          SamrenaCompareFn compare_fn, void* user_data);

// Find first element matching predicate
size_t samrena_vector_find_if(const SamrenaVector* vec, SamrenaPredicateFn predicate, 
                             void* user_data);

// Find last element matching comparator
size_t samrena_vector_find_last(const SamrenaVector* vec, const void* target,
                               SamrenaCompareFn compare_fn, void* user_data);

// Find all matching elements (returns count, fills indices array)
size_t samrena_vector_find_all(const SamrenaVector* vec, const void* target,
                              SamrenaCompareFn compare_fn, void* user_data,
                              size_t* indices, size_t max_indices);
```

### Existence Checking
```c
// Check if element exists (faster than find when index not needed)
bool samrena_vector_contains(const SamrenaVector* vec, const void* target,
                            SamrenaCompareFn compare_fn, void* user_data);

// Check if any element matches predicate
bool samrena_vector_any(const SamrenaVector* vec, SamrenaPredicateFn predicate,
                       void* user_data);

// Check if all elements match predicate
bool samrena_vector_all(const SamrenaVector* vec, SamrenaPredicateFn predicate,
                       void* user_data);

// Count elements matching criteria
size_t samrena_vector_count(const SamrenaVector* vec, const void* target,
                           SamrenaCompareFn compare_fn, void* user_data);

size_t samrena_vector_count_if(const SamrenaVector* vec, SamrenaPredicateFn predicate,
                              void* user_data);
```

### Range-Based Search
```c
// Search within specific range
size_t samrena_vector_find_range(const SamrenaVector* vec, size_t start, size_t end,
                                const void* target, SamrenaCompareFn compare_fn,
                                void* user_data);

// Find first element in range matching predicate  
size_t samrena_vector_find_if_range(const SamrenaVector* vec, size_t start, size_t end,
                                   SamrenaPredicateFn predicate, void* user_data);
```

### Optimized Search for Common Types
```c
// Built-in comparators for common types (no user_data needed)
size_t samrena_vector_find_int(const SamrenaVector* vec, int target);
size_t samrena_vector_find_float(const SamrenaVector* vec, float target);
size_t samrena_vector_find_double(const SamrenaVector* vec, double target);
size_t samrena_vector_find_string(const SamrenaVector* vec, const char* target);
size_t samrena_vector_find_ptr(const SamrenaVector* vec, const void* target);

// Memory-based comparison (bytewise)
size_t samrena_vector_find_bytes(const SamrenaVector* vec, const void* target);
```

### Binary Search (for sorted vectors)
```c
// Binary search on sorted vector (returns index or SIZE_MAX)
size_t samrena_vector_binary_search(const SamrenaVector* vec, const void* target,
                                   SamrenaCompareFn compare_fn, void* user_data);

// Binary search with insertion point (for maintaining sorted order)
typedef struct {
    bool found;           // True if element was found
    size_t index;         // Index of element if found, or insertion point
} SamrenaBinarySearchResult;

SamrenaBinarySearchResult samrena_vector_binary_search_insert_point(
    const SamrenaVector* vec, const void* target,
    SamrenaCompareFn compare_fn, void* user_data);

// Lower bound: first position where element could be inserted
size_t samrena_vector_lower_bound(const SamrenaVector* vec, const void* target,
                                 SamrenaCompareFn compare_fn, void* user_data);

// Upper bound: last position where element could be inserted
size_t samrena_vector_upper_bound(const SamrenaVector* vec, const void* target,
                                 SamrenaCompareFn compare_fn, void* user_data);
```

## Comparison Function Utilities

### Standard Comparators
```c
// Built-in comparison functions
int samrena_compare_int(const void* a, const void* b, void* user_data);
int samrena_compare_float(const void* a, const void* b, void* user_data);
int samrena_compare_double(const void* a, const void* b, void* user_data);
int samrena_compare_string(const void* a, const void* b, void* user_data);
int samrena_compare_ptr(const void* a, const void* b, void* user_data);
int samrena_compare_bytes(const void* a, const void* b, void* user_data);

// Reverse comparators
int samrena_compare_int_reverse(const void* a, const void* b, void* user_data);
int samrena_compare_float_reverse(const void* a, const void* b, void* user_data);
```

### Comparison Helpers
```c
// Wrap qsort-style comparator (no user_data) for use with samrena
typedef int (*QSortCompareFn)(const void* a, const void* b);

typedef struct {
    QSortCompareFn qsort_fn;
} SamrenaQSortWrapper;

int samrena_wrap_qsort_compare(const void* a, const void* b, void* user_data);

// Usage: SamrenaQSortWrapper wrapper = {my_qsort_comparator};
//        samrena_vector_find(vec, &target, samrena_wrap_qsort_compare, &wrapper);
```

## Design Considerations

### Performance Optimization
1. **Early Exit**: Stop searching as soon as match is found
2. **Cache Efficiency**: Sequential memory access patterns
3. **SIMD Opportunities**: Vectorized search for simple types
4. **Branch Prediction**: Optimize for common cases (element found vs not found)

### Element Access Strategy
```c
// Efficient element access during search
static inline const void* get_element_at_index(const SamrenaVector* vec, size_t index) {
    return (const char*)vec->data + (index * vec->element_size);
}

// Template for search loop
#define SAMRENA_SEARCH_LOOP(vec, start, end, condition, action) \
    do { \
        for (size_t i = (start); i < (end); i++) { \
            const void* element = get_element_at_index((vec), i); \
            if (condition) { \
                action; \
            } \
        } \
    } while(0)
```

### Memory Safety
- Validate vector and function pointers before use
- Ensure search ranges are within vector bounds
- Handle empty vectors gracefully
- Provide clear error indication (SIZE_MAX for not found)

## API Usage Examples

### Basic Search Example
```c
// Search for integer value
SamrenaVector* vec = create_int_vector();
size_t index = samrena_vector_find_int(vec, 42);
if (index != SIZE_MAX) {
    printf("Found 42 at index %zu\n", index);
}

// Custom comparison search
typedef struct { int id; char name[32]; } Person;

int compare_person_by_id(const void* a, const void* b, void* user_data) {
    const Person* p1 = (const Person*)a;
    const Person* p2 = (const Person*)b;
    return p1->id - p2->id;
}

Person target = {.id = 123};
size_t pos = samrena_vector_find(people_vec, &target, compare_person_by_id, NULL);
```

### Predicate Search Example
```c
// Find first even number
bool is_even(const void* element, void* user_data) {
    int value = *(const int*)element;
    return value % 2 == 0;
}

size_t first_even = samrena_vector_find_if(vec, is_even, NULL);

// Count elements in range
bool in_range(const void* element, void* user_data) {
    int value = *(const int*)element;
    int* range = (int*)user_data;
    return value >= range[0] && value <= range[1];
}

int range[] = {10, 20};
size_t count = samrena_vector_count_if(vec, in_range, range);
```

### Binary Search Example
```c
// Search sorted vector
SamrenaVector* sorted_vec = create_sorted_int_vector();
size_t index = samrena_vector_binary_search_int(sorted_vec, 42);

// Find insertion point for maintaining sort order
SamrenaBinarySearchResult result = samrena_vector_binary_search_insert_point(
    sorted_vec, &target, compare_int, NULL);

if (!result.found) {
    samrena_vector_insert(sorted_vec, result.index, &target);
}
```

## Performance Characteristics

### Time Complexity
- Linear search: O(n) average case, O(1) best case
- Binary search: O(log n) for sorted vectors
- Count operations: O(n) - must examine all elements
- Existence check: O(n) average, O(1) best case with early exit

### Space Complexity
- All search operations: O(1) additional space
- find_all: O(k) where k is number of matches (caller provides buffer)

## Testing Requirements
- Search correctness with various data types
- Performance comparison: linear vs binary search
- Edge cases: empty vectors, single elements, all matches
- Custom comparator validation
- Memory safety with invalid parameters

## Integration Notes
- Build upon existing vector validation
- Use established error handling patterns
- Consider SIMD optimizations for future enhancement
- Provide both type-safe and generic interfaces
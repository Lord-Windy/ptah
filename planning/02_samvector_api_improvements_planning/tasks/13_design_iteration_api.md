# Task 13: Design Iteration API

## Objective
Design a comprehensive iteration interface for Samvector that provides safe, efficient, and flexible ways to traverse and process vector elements, including callback-based iteration, iterator patterns, and range-based operations.

## Requirements
- Callback-based iteration with user data support
- Iterator-style traversal for C-style loops
- Range-based iteration for processing subsequences
- Forward and reverse iteration capabilities
- Safe iteration with bounds checking
- Support for early termination and filtered iteration
- Integration with existing error handling system

## Proposed API

### Callback-Based Iteration
```c
// Standard iteration callback signature
typedef void (*SamrenaVectorCallback)(void* element, size_t index, void* user_data);

// Iteration callback with early termination support
typedef bool (*SamrenaVectorCallbackEx)(void* element, size_t index, void* user_data);

// Basic iteration over all elements
void samrena_vector_foreach(SamrenaVector* vec, SamrenaVectorCallback callback, 
                           void* user_data);

// Iteration with early termination (callback returns false to stop)
size_t samrena_vector_foreach_ex(SamrenaVector* vec, SamrenaVectorCallbackEx callback, 
                                void* user_data);

// Reverse iteration
void samrena_vector_foreach_reverse(SamrenaVector* vec, SamrenaVectorCallback callback, 
                                   void* user_data);

size_t samrena_vector_foreach_reverse_ex(SamrenaVector* vec, 
                                        SamrenaVectorCallbackEx callback, 
                                        void* user_data);
```

### Range-Based Iteration
```c
// Iterate over specific range
void samrena_vector_foreach_range(SamrenaVector* vec, size_t start, size_t end,
                                 SamrenaVectorCallback callback, void* user_data);

size_t samrena_vector_foreach_range_ex(SamrenaVector* vec, size_t start, size_t end,
                                      SamrenaVectorCallbackEx callback, void* user_data);

// Iterate with step size (every nth element)
void samrena_vector_foreach_step(SamrenaVector* vec, size_t step,
                                SamrenaVectorCallback callback, void* user_data);

// Iterate over elements matching predicate
void samrena_vector_foreach_if(SamrenaVector* vec, SamrenaPredicateFn predicate,
                              SamrenaVectorCallback callback, void* user_data);
```

### Iterator Interface
```c
// Iterator structure for C-style loops
typedef struct {
    SamrenaVector* vector;
    size_t current_index;
    size_t end_index;
    int direction;        // 1 for forward, -1 for reverse
    bool valid;
} SamrenaVectorIterator;

// Create iterator for full vector
SamrenaVectorIterator samrena_vector_begin(SamrenaVector* vec);
SamrenaVectorIterator samrena_vector_end(SamrenaVector* vec);

// Create reverse iterator
SamrenaVectorIterator samrena_vector_rbegin(SamrenaVector* vec);
SamrenaVectorIterator samrena_vector_rend(SamrenaVector* vec);

// Create range iterator
SamrenaVectorIterator samrena_vector_iter_range(SamrenaVector* vec, 
                                               size_t start, size_t end);

// Iterator operations
bool samrena_vector_iter_valid(const SamrenaVectorIterator* iter);
void* samrena_vector_iter_current(const SamrenaVectorIterator* iter);
size_t samrena_vector_iter_index(const SamrenaVectorIterator* iter);
void samrena_vector_iter_next(SamrenaVectorIterator* iter);
void samrena_vector_iter_prev(SamrenaVectorIterator* iter);

// Iterator comparison
bool samrena_vector_iter_equals(const SamrenaVectorIterator* a, 
                               const SamrenaVectorIterator* b);
```

### Const-Correct Iteration
```c
// Read-only callback signature
typedef void (*SamrenaVectorConstCallback)(const void* element, size_t index, 
                                          void* user_data);
typedef bool (*SamrenaVectorConstCallbackEx)(const void* element, size_t index, 
                                            void* user_data);

// Const iteration functions
void samrena_vector_foreach_const(const SamrenaVector* vec, 
                                 SamrenaVectorConstCallback callback, 
                                 void* user_data);

size_t samrena_vector_foreach_const_ex(const SamrenaVector* vec, 
                                      SamrenaVectorConstCallbackEx callback, 
                                      void* user_data);

// Const iterator
typedef struct {
    const SamrenaVector* vector;
    size_t current_index;
    size_t end_index;
    int direction;
    bool valid;
} SamrenaVectorConstIterator;

SamrenaVectorConstIterator samrena_vector_cbegin(const SamrenaVector* vec);
const void* samrena_vector_const_iter_current(const SamrenaVectorConstIterator* iter);
```

### Functional-Style Operations
```c
// Transform each element in place
void samrena_vector_transform(SamrenaVector* vec, 
                             void (*transform_fn)(void* element, void* user_data),
                             void* user_data);

// Map to new vector (requires compatible element types)
SamrenaVector* samrena_vector_map(const SamrenaVector* src_vec,
                                 size_t dest_element_size,
                                 void (*map_fn)(const void* src, void* dest, void* user_data),
                                 void* user_data);

// Reduce vector to single value
bool samrena_vector_reduce(const SamrenaVector* vec, void* accumulator,
                          void (*reduce_fn)(void* acc, const void* element, void* user_data),
                          void* user_data);

// Filter elements matching predicate to new vector
SamrenaVector* samrena_vector_filter(const SamrenaVector* src_vec,
                                    SamrenaPredicateFn predicate,
                                    void* user_data);
```

## Design Considerations

### Performance Optimization
1. **Cache-Friendly Access**: Sequential memory access patterns
2. **Inlined Iterator Operations**: Fast iteration with minimal overhead
3. **Early Termination**: Efficient short-circuit evaluation
4. **Batch Processing**: Encourage bulk operations over single-element loops

### Memory Safety
```c
// Iterator validation
static inline bool validate_iterator(const SamrenaVectorIterator* iter) {
    return iter && iter->vector && iter->valid && 
           iter->current_index < iter->vector->size;
}

// Safe element access during iteration
static inline void* safe_iter_element(const SamrenaVectorIterator* iter) {
    if (!validate_iterator(iter)) return NULL;
    return (char*)iter->vector->data + (iter->current_index * iter->vector->element_size);
}
```

### Thread Safety Considerations
```c
// Iterator invalidation detection
typedef struct {
    SamrenaVector* vector;
    uint64_t vector_version;  // Incremented on structural changes
    size_t current_index;
    bool valid;
} SamrenaVectorSafeIterator;

// Check if iterator is still valid (vector hasn't changed)
bool samrena_vector_iter_is_current(const SamrenaVectorSafeIterator* iter);
```

## API Usage Examples

### Basic Iteration Example
```c
// Print all elements
void print_int(void* element, size_t index, void* user_data) {
    printf("[%zu] = %d\n", index, *(int*)element);
}

SamrenaVector* vec = create_int_vector();
samrena_vector_foreach(vec, print_int, NULL);
```

### Iterator Loop Example
```c
// C-style iterator loop
for (SamrenaVectorIterator it = samrena_vector_begin(vec);
     samrena_vector_iter_valid(&it);
     samrena_vector_iter_next(&it)) {
    
    int* element = (int*)samrena_vector_iter_current(&it);
    size_t index = samrena_vector_iter_index(&it);
    printf("[%zu] = %d\n", index, *element);
}
```

### Early Termination Example
```c
// Find first element greater than threshold
typedef struct {
    int threshold;
    int* result;
    size_t* result_index;
} FindContext;

bool find_greater_than(void* element, size_t index, void* user_data) {
    FindContext* ctx = (FindContext*)user_data;
    int value = *(int*)element;
    
    if (value > ctx->threshold) {
        *ctx->result = value;
        *ctx->result_index = index;
        return false;  // Stop iteration
    }
    return true;  // Continue
}

FindContext ctx = {.threshold = 50, .result = &result, .result_index = &index};
size_t processed = samrena_vector_foreach_ex(vec, find_greater_than, &ctx);
```

### Functional Programming Example
```c
// Transform all elements (square them)
void square_int(void* element, void* user_data) {
    int* value = (int*)element;
    *value = (*value) * (*value);
}

samrena_vector_transform(vec, square_int, NULL);

// Filter even numbers
bool is_even(const void* element, void* user_data) {
    return (*(const int*)element) % 2 == 0;
}

SamrenaVector* even_numbers = samrena_vector_filter(vec, is_even, NULL);

// Sum all elements
void sum_reduce(void* accumulator, const void* element, void* user_data) {
    *(int*)accumulator += *(const int*)element;
}

int sum = 0;
samrena_vector_reduce(vec, &sum, sum_reduce, NULL);
```

### Range Iteration Example
```c
// Process middle third of vector
size_t start = vec->size / 3;
size_t end = (vec->size * 2) / 3;

samrena_vector_foreach_range(vec, start, end, process_element, &context);

// Process every other element
samrena_vector_foreach_step(vec, 2, process_element, &context);
```

## Performance Characteristics

### Time Complexity
- `foreach`: O(n) linear traversal
- Iterator operations: O(1) per step
- Functional operations: O(n) for single pass operations
- Range iteration: O(k) where k is range size

### Space Complexity
- Callback iteration: O(1) additional space
- Iterator: O(1) space per iterator
- Map/filter operations: O(n) for result vector

## Error Handling Strategy

### Validation Requirements
- Check vector and callback pointers before iteration
- Validate iterator state before each operation
- Handle empty vectors gracefully
- Detect iterator invalidation from vector modifications

### Error Reporting
```c
// Iterator-specific errors
typedef enum {
    SAMRENA_ITER_SUCCESS = 0,
    SAMRENA_ITER_ERROR_INVALID = -1,
    SAMRENA_ITER_ERROR_OUT_OF_BOUNDS = -2,
    SAMRENA_ITER_ERROR_MODIFIED = -3
} SamrenaIterError;

// Get iterator error state
SamrenaIterError samrena_vector_iter_error(const SamrenaVectorIterator* iter);
```

## Testing Requirements
- Iteration correctness with various data types
- Iterator bounds checking and safety
- Performance comparison with manual loops
- Early termination functionality
- Const-correctness validation
- Memory safety with invalid iterators

## Integration Notes
- Build upon existing element access functions
- Use vector validation from error handling system
- Compatible with both owned and shared arena vectors
- Provide foundation for future parallel iteration support
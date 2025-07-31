# Task 14: Implement Iteration

## Objective
Implement the iteration API designed in Task 13, providing efficient and safe vector traversal capabilities with callback-based iteration, iterators, and functional programming operations.

## Dependencies
- Task 13: Design Iteration API (must be completed)
- Element access functions from Task 02
- Vector validation from Task 08
- Search functions from Task 12

## Implementation Plan

### 1. Core Iteration Infrastructure
```c
// samrena_vector_iteration.c - Core iteration implementation

#include <assert.h>
#include <string.h>

// Internal helper for safe element access during iteration
static inline void* iter_get_element(const SamrenaVector* vec, size_t index) {
    if (!vec || index >= vec->size) return NULL;
    return (char*)vec->data + (index * vec->element_size);
}

static inline const void* iter_get_element_const(const SamrenaVector* vec, size_t index) {
    if (!vec || index >= vec->size) return NULL;
    return (const char*)vec->data + (index * vec->element_size);
}

// Validate iteration parameters
static SamrenaError validate_iteration_params(const SamrenaVector* vec, const void* callback) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!callback) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Callback function is NULL", callback);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    return SAMRENA_SUCCESS;
}
```

### 2. Basic Callback Iteration
```c
void samrena_vector_foreach(SamrenaVector* vec, SamrenaVectorCallback callback, 
                           void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        void* element = iter_get_element(vec, i);
        if (element) {
            callback(element, i, user_data);
        }
    }
}

size_t samrena_vector_foreach_ex(SamrenaVector* vec, SamrenaVectorCallbackEx callback, 
                                void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return 0;
    }
    
    size_t processed = 0;
    
    for (size_t i = 0; i < vec->size; i++) {
        void* element = iter_get_element(vec, i);
        if (element) {
            bool continue_iteration = callback(element, i, user_data);
            processed++;
            
            if (!continue_iteration) {
                break;
            }
        }
    }
    
    return processed;
}

void samrena_vector_foreach_reverse(SamrenaVector* vec, SamrenaVectorCallback callback, 
                                   void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return;
    }
    
    for (size_t i = vec->size; i > 0; i--) {
        void* element = iter_get_element(vec, i - 1);
        if (element) {
            callback(element, i - 1, user_data);
        }
    }
}

size_t samrena_vector_foreach_reverse_ex(SamrenaVector* vec, 
                                        SamrenaVectorCallbackEx callback, 
                                        void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return 0;
    }
    
    size_t processed = 0;
    
    for (size_t i = vec->size; i > 0; i--) {
        void* element = iter_get_element(vec, i - 1);
        if (element) {
            bool continue_iteration = callback(element, i - 1, user_data);
            processed++;
            
            if (!continue_iteration) {
                break;
            }
        }
    }
    
    return processed;
}
```

### 3. Const-Correct Iteration
```c
void samrena_vector_foreach_const(const SamrenaVector* vec, 
                                 SamrenaVectorConstCallback callback, 
                                 void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = iter_get_element_const(vec, i);
        if (element) {
            callback(element, i, user_data);
        }
    }
}

size_t samrena_vector_foreach_const_ex(const SamrenaVector* vec, 
                                      SamrenaVectorConstCallbackEx callback, 
                                      void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return 0;
    }
    
    size_t processed = 0;
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = iter_get_element_const(vec, i);
        if (element) {
            bool continue_iteration = callback(element, i, user_data);
            processed++;
            
            if (!continue_iteration) {
                break;
            }
        }
    }
    
    return processed;
}
```

### 4. Range-Based Iteration
```c
void samrena_vector_foreach_range(SamrenaVector* vec, size_t start, size_t end,
                                 SamrenaVectorCallback callback, void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return;
    }
    
    // Validate and clamp range
    if (start >= vec->size) return;
    if (end > vec->size) end = vec->size;
    if (start >= end) return;
    
    for (size_t i = start; i < end; i++) {
        void* element = iter_get_element(vec, i);
        if (element) {
            callback(element, i, user_data);
        }
    }
}

size_t samrena_vector_foreach_range_ex(SamrenaVector* vec, size_t start, size_t end,
                                      SamrenaVectorCallbackEx callback, void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS) {
        return 0;
    }
    
    if (start >= vec->size) return 0;
    if (end > vec->size) end = vec->size;
    if (start >= end) return 0;
    
    size_t processed = 0;
    
    for (size_t i = start; i < end; i++) {
        void* element = iter_get_element(vec, i);
        if (element) {
            bool continue_iteration = callback(element, i, user_data);
            processed++;
            
            if (!continue_iteration) {
                break;
            }
        }
    }
    
    return processed;
}

void samrena_vector_foreach_step(SamrenaVector* vec, size_t step,
                                SamrenaVectorCallback callback, void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS || step == 0) {
        return;
    }
    
    for (size_t i = 0; i < vec->size; i += step) {
        void* element = iter_get_element(vec, i);
        if (element) {
            callback(element, i, user_data);
        }
    }
}

void samrena_vector_foreach_if(SamrenaVector* vec, SamrenaPredicateFn predicate,
                              SamrenaVectorCallback callback, void* user_data) {
    if (validate_iteration_params(vec, callback) != SAMRENA_SUCCESS || !predicate) {
        return;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        void* element = iter_get_element(vec, i);
        if (element && predicate(element, user_data)) {
            callback(element, i, user_data);
        }
    }
}
```

### 5. Iterator Implementation
```c
// Iterator validation
static inline bool validate_iterator(const SamrenaVectorIterator* iter) {
    return iter && iter->vector && iter->valid && 
           iter->current_index <= iter->vector->size;
}

SamrenaVectorIterator samrena_vector_begin(SamrenaVector* vec) {
    SamrenaVectorIterator iter = {0};
    
    if (validate_vector(vec) == SAMRENA_SUCCESS) {
        iter.vector = vec;
        iter.current_index = 0;
        iter.end_index = vec->size;
        iter.direction = 1;
        iter.valid = (vec->size > 0);
    }
    
    return iter;
}

SamrenaVectorIterator samrena_vector_end(SamrenaVector* vec) {
    SamrenaVectorIterator iter = {0};
    
    if (validate_vector(vec) == SAMRENA_SUCCESS) {
        iter.vector = vec;
        iter.current_index = vec->size;
        iter.end_index = vec->size;
        iter.direction = 1;
        iter.valid = false;  // End iterator is not valid for dereferencing
    }
    
    return iter;
}

SamrenaVectorIterator samrena_vector_rbegin(SamrenaVector* vec) {
    SamrenaVectorIterator iter = {0};
    
    if (validate_vector(vec) == SAMRENA_SUCCESS && vec->size > 0) {
        iter.vector = vec;
        iter.current_index = vec->size - 1;
        iter.end_index = 0;
        iter.direction = -1;
        iter.valid = true;
    }
    
    return iter;
}

SamrenaVectorIterator samrena_vector_rend(SamrenaVector* vec) {
    SamrenaVectorIterator iter = {0};
    
    if (validate_vector(vec) == SAMRENA_SUCCESS) {
        iter.vector = vec;
        iter.current_index = SIZE_MAX;  // Before first element
        iter.end_index = 0;
        iter.direction = -1;
        iter.valid = false;
    }
    
    return iter;
}

SamrenaVectorIterator samrena_vector_iter_range(SamrenaVector* vec, 
                                               size_t start, size_t end) {
    SamrenaVectorIterator iter = {0};
    
    if (validate_vector(vec) == SAMRENA_SUCCESS) {
        if (start >= vec->size) start = vec->size;
        if (end > vec->size) end = vec->size;
        
        iter.vector = vec;
        iter.current_index = start;
        iter.end_index = end;
        iter.direction = 1;
        iter.valid = (start < end);
    }
    
    return iter;
}

bool samrena_vector_iter_valid(const SamrenaVectorIterator* iter) {
    if (!validate_iterator(iter)) return false;
    
    if (iter->direction > 0) {
        return iter->current_index < iter->end_index;
    } else {
        return iter->current_index != SIZE_MAX && iter->current_index >= iter->end_index;
    }
}

void* samrena_vector_iter_current(const SamrenaVectorIterator* iter) {
    if (!samrena_vector_iter_valid(iter)) return NULL;
    return iter_get_element(iter->vector, iter->current_index);
}

size_t samrena_vector_iter_index(const SamrenaVectorIterator* iter) {
    if (!validate_iterator(iter)) return SIZE_MAX;
    return iter->current_index;
}

void samrena_vector_iter_next(SamrenaVectorIterator* iter) {
    if (!validate_iterator(iter)) return;
    
    if (iter->direction > 0) {
        iter->current_index++;
        iter->valid = (iter->current_index < iter->end_index);
    } else {
        if (iter->current_index == 0 || iter->current_index == SIZE_MAX) {
            iter->current_index = SIZE_MAX;
            iter->valid = false;
        } else {
            iter->current_index--;
            iter->valid = (iter->current_index >= iter->end_index);
        }
    }
}

void samrena_vector_iter_prev(SamrenaVectorIterator* iter) {
    if (!validate_iterator(iter)) return;
    
    if (iter->direction > 0) {
        if (iter->current_index == 0) {
            iter->valid = false;
        } else {
            iter->current_index--;
            iter->valid = true;
        }
    } else {
        iter->current_index++;
        iter->valid = (iter->current_index < iter->vector->size && 
                      iter->current_index >= iter->end_index);
    }
}

bool samrena_vector_iter_equals(const SamrenaVectorIterator* a, 
                               const SamrenaVectorIterator* b) {
    if (!a || !b) return false;
    return (a->vector == b->vector && 
            a->current_index == b->current_index &&
            a->direction == b->direction);
}
```

### 6. Const Iterator Implementation
```c
SamrenaVectorConstIterator samrena_vector_cbegin(const SamrenaVector* vec) {
    SamrenaVectorConstIterator iter = {0};
    
    if (validate_vector(vec) == SAMRENA_SUCCESS) {
        iter.vector = vec;
        iter.current_index = 0;
        iter.end_index = vec->size;
        iter.direction = 1;
        iter.valid = (vec->size > 0);
    }
    
    return iter;
}

const void* samrena_vector_const_iter_current(const SamrenaVectorConstIterator* iter) {
    if (!iter || !iter->valid || !iter->vector) return NULL;
    if (iter->current_index >= iter->vector->size) return NULL;
    
    return iter_get_element_const(iter->vector, iter->current_index);
}
```

### 7. Functional Programming Operations
```c
void samrena_vector_transform(SamrenaVector* vec, 
                             void (*transform_fn)(void* element, void* user_data),
                             void* user_data) {
    if (validate_iteration_params(vec, transform_fn) != SAMRENA_SUCCESS) {
        return;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        void* element = iter_get_element(vec, i);
        if (element) {
            transform_fn(element, user_data);
        }
    }
}

SamrenaVector* samrena_vector_map(const SamrenaVector* src_vec,
                                 size_t dest_element_size,
                                 void (*map_fn)(const void* src, void* dest, void* user_data),
                                 void* user_data) {
    if (validate_iteration_params(src_vec, map_fn) != SAMRENA_SUCCESS) {
        return NULL;
    }
    
    if (dest_element_size == 0) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_SIZE, "Destination element size is zero", NULL);
        return NULL;
    }
    
    SamrenaVector* dest_vec = samrena_vector_init_owned(dest_element_size, src_vec->size);
    if (!dest_vec) return NULL;
    
    // Pre-allocate space for all elements
    if (samrena_vector_reserve_auto(dest_vec, src_vec->size) != SAMRENA_SUCCESS) {
        samrena_vector_destroy(dest_vec);
        return NULL;
    }
    
    for (size_t i = 0; i < src_vec->size; i++) {
        const void* src_element = iter_get_element_const(src_vec, i);
        if (src_element) {
            // Allocate space for destination element
            char dest_element[dest_element_size];
            map_fn(src_element, dest_element, user_data);
            
            // Add to destination vector
            if (samrena_vector_push_auto(dest_vec, dest_element) != SAMRENA_SUCCESS) {
                samrena_vector_destroy(dest_vec);
                return NULL;
            }
        }
    }
    
    return dest_vec;
}

bool samrena_vector_reduce(const SamrenaVector* vec, void* accumulator,
                          void (*reduce_fn)(void* acc, const void* element, void* user_data),
                          void* user_data) {
    if (validate_iteration_params(vec, reduce_fn) != SAMRENA_SUCCESS) {
        return false;
    }
    
    if (!accumulator) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Accumulator is NULL", accumulator);
        return false;
    }
    
    for (size_t i = 0; i < vec->size; i++) {
        const void* element = iter_get_element_const(vec, i);
        if (element) {
            reduce_fn(accumulator, element, user_data);
        }
    }
    
    return true;
}

SamrenaVector* samrena_vector_filter(const SamrenaVector* src_vec,
                                    SamrenaPredicateFn predicate,
                                    void* user_data) {
    if (validate_iteration_params(src_vec, predicate) != SAMRENA_SUCCESS) {
        return NULL;
    }
    
    SamrenaVector* dest_vec = samrena_vector_init_owned(src_vec->element_size, 
                                                       src_vec->size / 2);  // Estimate
    if (!dest_vec) return NULL;
    
    for (size_t i = 0; i < src_vec->size; i++) {
        const void* src_element = iter_get_element_const(src_vec, i);
        if (src_element && predicate(src_element, user_data)) {
            if (samrena_vector_push_auto(dest_vec, src_element) != SAMRENA_SUCCESS) {
                samrena_vector_destroy(dest_vec);
                return NULL;
            }
        }
    }
    
    return dest_vec;
}
```

## Performance Optimizations

### Optimized Type-Specific Iteration
```c
// Specialized iteration for common types to avoid function call overhead
static inline void foreach_int_inline(SamrenaVector* vec, 
                                     void (*callback)(int* element, size_t index)) {
    if (vec->element_size != sizeof(int)) return;
    
    int* data = (int*)vec->data;
    for (size_t i = 0; i < vec->size; i++) {
        callback(&data[i], i);
    }
}

// Macro for type-safe iteration
#define SAMRENA_FOREACH_TYPE(vec, type, callback) \
    do { \
        if ((vec)->element_size == sizeof(type)) { \
            type* data = (type*)(vec)->data; \
            for (size_t i = 0; i < (vec)->size; i++) { \
                callback(&data[i], i); \
            } \
        } \
    } while(0)
```

### Cache-Optimized Access
```c
// Prefetch next element for better cache performance
#ifdef SAMRENA_USE_PREFETCH
static inline void prefetch_next_element(const SamrenaVector* vec, size_t current_index) {
    if (current_index + 1 < vec->size) {
        const char* next_element = (const char*)vec->data + 
                                  ((current_index + 1) * vec->element_size);
        __builtin_prefetch(next_element, 0, 1);  // Read prefetch, temporal locality
    }
}
#endif
```

## Testing Strategy

### Unit Tests
```c
void test_basic_iteration() {
    SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 5);
    int values[] = {1, 2, 3, 4, 5};
    samrena_vector_push_array(vec, values, 5);
    
    // Test basic iteration
    int sum = 0;
    void sum_callback(void* element, size_t index, void* user_data) {
        *(int*)user_data += *(int*)element;
    }
    
    samrena_vector_foreach(vec, sum_callback, &sum);
    assert(sum == 15);
    
    samrena_vector_destroy(vec);
}

void test_iterator_loop() {
    SamrenaVector* vec = create_test_vector();
    
    int count = 0;
    for (SamrenaVectorIterator it = samrena_vector_begin(vec);
         samrena_vector_iter_valid(&it);
         samrena_vector_iter_next(&it)) {
        
        void* element = samrena_vector_iter_current(&it);
        assert(element != NULL);
        count++;
    }
    
    assert(count == samrena_vector_size(vec));
}

void test_early_termination() {
    SamrenaVector* vec = create_test_vector();
    
    bool find_target(void* element, size_t index, void* user_data) {
        int target = *(int*)user_data;
        return *(int*)element != target;  // Continue until target found
    }
    
    int target = 42;
    size_t processed = samrena_vector_foreach_ex(vec, find_target, &target);
    // Should process elements until target is found
}
```

### Performance Benchmarks
```c
void benchmark_iteration_methods() {
    const size_t count = 1000000;
    SamrenaVector* vec = create_large_int_vector(count);
    
    // Benchmark callback iteration
    clock_t start = clock();
    samrena_vector_foreach(vec, sum_callback, &sum1);
    clock_t callback_time = clock() - start;
    
    // Benchmark iterator loop
    start = clock();
    int sum2 = 0;
    for (SamrenaVectorIterator it = samrena_vector_begin(vec);
         samrena_vector_iter_valid(&it);
         samrena_vector_iter_next(&it)) {
        sum2 += *(int*)samrena_vector_iter_current(&it);
    }
    clock_t iterator_time = clock() - start;
    
    // Benchmark direct access
    start = clock();
    int sum3 = 0;
    for (size_t i = 0; i < vec->size; i++) {
        sum3 += *(int*)samrena_vector_at_unchecked(vec, i);
    }
    clock_t direct_time = clock() - start;
    
    printf("Callback: %ld, Iterator: %ld, Direct: %ld\n", 
           callback_time, iterator_time, direct_time);
}
```

## Integration Notes
- Uses existing vector validation and element access
- Compatible with error handling system
- Works with both owned and shared arena vectors
- Provides foundation for future parallel iteration
- Maintains performance while adding safety checks
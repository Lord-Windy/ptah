# Task 10: Implement Bulk Operations

## Objective
Implement the bulk operations API designed in Task 09, focusing on high-performance batch operations that minimize memory allocations and maximize throughput.

## Dependencies
- Task 09: Design Bulk Operations API (must be completed)
- Capacity management functions from Task 04
- Error handling system from Task 08

## Implementation Plan

### 1. Helper Functions for Bulk Operations
```c
// samrena_vector_bulk.c - Internal helpers

// Calculate optimal capacity for bulk insertion
static size_t calculate_bulk_capacity(const SamrenaVector* vec, size_t additional) {
    size_t required = vec->size + additional;
    if (required <= vec->capacity) return vec->capacity;
    
    // Use smart growth that considers bulk size
    size_t growth_by_factor = (size_t)(vec->capacity * vec->growth_factor);
    size_t growth_by_minimum = vec->capacity + vec->min_growth;
    size_t growth_with_bulk = vec->capacity + additional;
    
    // Choose the largest growth to minimize future reallocations
    size_t new_capacity = growth_by_factor;
    if (growth_by_minimum > new_capacity) new_capacity = growth_by_minimum;
    if (growth_with_bulk > new_capacity) new_capacity = growth_with_bulk;
    
    return new_capacity;
}

// Shift elements right to make space for insertion
static void shift_elements_right(SamrenaVector* vec, size_t index, size_t count) {
    if (index >= vec->size || count == 0) return;
    
    char* data = (char*)vec->data;
    size_t element_size = vec->element_size;
    
    // Calculate source and destination pointers
    char* src = data + (index * element_size);
    char* dst = data + ((index + count) * element_size);
    size_t move_size = (vec->size - index) * element_size;
    
    // Use memmove for overlapping memory regions
    memmove(dst, src, move_size);
}

// Shift elements left to close gaps after removal
static void shift_elements_left(SamrenaVector* vec, size_t index, size_t count) {
    if (index >= vec->size || count == 0) return;
    
    char* data = (char*)vec->data;
    size_t element_size = vec->element_size;
    
    // Calculate source and destination pointers
    char* dst = data + (index * element_size);
    char* src = data + ((index + count) * element_size);
    size_t elements_after = vec->size - (index + count);
    size_t move_size = elements_after * element_size;
    
    if (elements_after > 0) {
        memmove(dst, src, move_size);
    }
}
```

### 2. Bulk Insertion Implementation
```c
int samrena_vector_push_array(SamrenaVector* vec, const void* elements, size_t count) {
    if (count == 0) return SAMRENA_SUCCESS;
    
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    err = validate_element_pointer(elements);
    if (err != SAMRENA_SUCCESS) return err;
    
    // Ensure sufficient capacity
    size_t required_capacity = vec->size + count;
    if (required_capacity > vec->capacity) {
        size_t new_capacity = calculate_bulk_capacity(vec, count);
        err = samrena_vector_reserve_auto(vec, new_capacity);
        if (err != SAMRENA_SUCCESS) return err;
    }
    
    // Copy elements to end of vector
    char* dst = (char*)vec->data + (vec->size * vec->element_size);
    memcpy(dst, elements, count * vec->element_size);
    vec->size += count;
    
    return SAMRENA_SUCCESS;
}

int samrena_vector_insert_array(SamrenaVector* vec, size_t index, 
                               const void* elements, size_t count) {
    if (count == 0) return SAMRENA_SUCCESS;
    
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    // Allow insertion at end (index == size)
    if (index > vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "Insert index out of bounds", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    err = validate_element_pointer(elements);
    if (err != SAMRENA_SUCCESS) return err;
    
    // Handle insertion at end as push_array (optimization)
    if (index == vec->size) {
        return samrena_vector_push_array(vec, elements, count);
    }
    
    // Ensure sufficient capacity
    size_t required_capacity = vec->size + count;
    if (required_capacity > vec->capacity) {
        size_t new_capacity = calculate_bulk_capacity(vec, count);
        err = samrena_vector_reserve_auto(vec, new_capacity);
        if (err != SAMRENA_SUCCESS) return err;
    }
    
    // Shift existing elements right
    shift_elements_right(vec, index, count);
    
    // Insert new elements
    char* dst = (char*)vec->data + (index * vec->element_size);
    memcpy(dst, elements, count * vec->element_size);
    vec->size += count;
    
    return SAMRENA_SUCCESS;
}

int samrena_vector_insert(SamrenaVector* vec, size_t index, const void* element) {
    return samrena_vector_insert_array(vec, index, element, 1);
}
```

### 3. Vector Append Operations
```c
int samrena_vector_append_vector(SamrenaVector* dest, const SamrenaVector* src) {
    SamrenaError err = validate_vector(dest);
    if (err != SAMRENA_SUCCESS) return err;
    
    err = validate_vector(src);
    if (err != SAMRENA_SUCCESS) return err;
    
    // Check element size compatibility
    if (dest->element_size != src->element_size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_OPERATION, 
                         "Element sizes don't match", dest);
        return SAMRENA_ERROR_INVALID_OPERATION;
    }
    
    if (src->size == 0) return SAMRENA_SUCCESS;
    
    return samrena_vector_push_array(dest, src->data, src->size);
}

int samrena_vector_insert_vector(SamrenaVector* dest, size_t index, 
                                const SamrenaVector* src) {
    SamrenaError err = validate_vector(dest);
    if (err != SAMRENA_SUCCESS) return err;
    
    err = validate_vector(src);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (dest->element_size != src->element_size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_OPERATION, 
                         "Element sizes don't match", dest);
        return SAMRENA_ERROR_INVALID_OPERATION;
    }
    
    if (src->size == 0) return SAMRENA_SUCCESS;
    
    return samrena_vector_insert_array(dest, index, src->data, src->size);
}
```

### 4. Bulk Removal Implementation
```c
int samrena_vector_remove(SamrenaVector* vec, size_t index) {
    return samrena_vector_remove_range(vec, index, 1);
}

int samrena_vector_remove_range(SamrenaVector* vec, size_t start, size_t count) {
    if (count == 0) return SAMRENA_SUCCESS;
    
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (start >= vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "Remove start index out of bounds", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    // Clamp count to available elements
    if (start + count > vec->size) {
        count = vec->size - start;
    }
    
    // Shift elements left to close the gap
    shift_elements_left(vec, start, count);
    vec->size -= count;
    
    return SAMRENA_SUCCESS;
}

int samrena_vector_remove_slice(SamrenaVector* vec, size_t start, size_t end) {
    if (start >= end) return SAMRENA_SUCCESS;
    
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (start >= vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_OUT_OF_BOUNDS, "Slice start out of bounds", vec);
        return SAMRENA_ERROR_OUT_OF_BOUNDS;
    }
    
    // Clamp end to vector size
    if (end > vec->size) end = vec->size;
    
    return samrena_vector_remove_range(vec, start, end - start);
}

int samrena_vector_remove_swap(SamrenaVector* vec, size_t index) {
    SamrenaError err = validate_index(vec, index);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (vec->size == 1) {
        vec->size = 0;
        return SAMRENA_SUCCESS;
    }
    
    // Swap with last element if not already the last
    if (index != vec->size - 1) {
        char* element_ptr = (char*)vec->data + (index * vec->element_size);
        char* last_ptr = (char*)vec->data + ((vec->size - 1) * vec->element_size);
        
        // Perform the swap using a temporary buffer
        char temp[vec->element_size];
        memcpy(temp, element_ptr, vec->element_size);
        memcpy(element_ptr, last_ptr, vec->element_size);
        memcpy(last_ptr, temp, vec->element_size);
    }
    
    vec->size--;
    return SAMRENA_SUCCESS;
}
```

### 5. Predicate-Based Removal
```c
size_t samrena_vector_remove_if(SamrenaVector* vec, SamrenaVectorPredicate pred, 
                               void* user_data) {
    if (!vec || !pred) return 0;
    
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return 0;
    
    if (vec->size == 0) return 0;
    
    size_t write_index = 0;
    size_t removed_count = 0;
    
    // Single pass: move elements that don't match predicate
    for (size_t read_index = 0; read_index < vec->size; read_index++) {
        char* element = (char*)vec->data + (read_index * vec->element_size);
        
        if (pred(element, user_data)) {
            // Element matches predicate - skip (remove)
            removed_count++;
        } else {
            // Element doesn't match - keep
            if (write_index != read_index) {
                char* write_ptr = (char*)vec->data + (write_index * vec->element_size);
                memcpy(write_ptr, element, vec->element_size);
            }
            write_index++;
        }
    }
    
    vec->size = write_index;
    return removed_count;
}
```

### 6. Array Interoperability
```c
int samrena_vector_to_array(const SamrenaVector* vec, void* array, 
                           size_t array_capacity) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return err;
    
    if (!array) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Array pointer is NULL", array);
        return SAMRENA_ERROR_NULL_POINTER;
    }
    
    if (array_capacity < vec->size) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INSUFFICIENT_CAPACITY, 
                         "Array capacity insufficient", vec);
        return SAMRENA_ERROR_INSUFFICIENT_CAPACITY;
    }
    
    if (vec->size > 0) {
        memcpy(array, vec->data, vec->size * vec->element_size);
    }
    
    return SAMRENA_SUCCESS;
}

SamrenaVector* samrena_vector_from_array(const void* array, size_t count, 
                                        size_t element_size) {
    if (!array && count > 0) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_NULL_POINTER, "Array pointer is NULL", array);
        return NULL;
    }
    
    if (element_size == 0) {
        SAMRENA_SET_ERROR(SAMRENA_ERROR_INVALID_SIZE, "Element size is zero", NULL);
        return NULL;
    }
    
    SamrenaVector* vec = samrena_vector_init_owned(element_size, count);
    if (!vec) return NULL;
    
    if (count > 0) {
        int result = samrena_vector_push_array(vec, array, count);
        if (result != SAMRENA_SUCCESS) {
            samrena_vector_destroy(vec);
            return NULL;
        }
    }
    
    return vec;
}

const void* samrena_vector_slice(const SamrenaVector* vec, size_t start, size_t count) {
    SamrenaError err = validate_vector(vec);
    if (err != SAMRENA_SUCCESS) return NULL;
    
    if (start >= vec->size || count == 0) return NULL;
    if (start + count > vec->size) return NULL;
    
    return (const char*)vec->data + (start * vec->element_size);
}
```

## Performance Optimizations

### SIMD-Optimized Operations (Future Enhancement)
```c
// For large arrays with simple types, consider SIMD operations
#ifdef SAMRENA_USE_SIMD
static void optimized_copy_int32(void* dst, const void* src, size_t count) {
    // Use vectorized copy for large int32 arrays
    if (count >= 8) {
        // SIMD implementation here
    } else {
        memcpy(dst, src, count * sizeof(int32_t));
    }
}
#endif
```

### Batch Allocation Strategy
```c
// For multiple bulk operations, provide transaction-like batching
typedef struct {
    SamrenaVector* vector;
    size_t reserved_capacity;
    bool in_batch;
} SamrenaVectorBatch;

SamrenaVectorBatch samrena_vector_begin_batch(SamrenaVector* vec, size_t estimated_additions) {
    // Pre-allocate for estimated operations
    SamrenaVectorBatch batch = {0};
    batch.vector = vec;
    batch.reserved_capacity = vec->capacity;
    
    if (estimated_additions > 0) {
        size_t target_capacity = calculate_bulk_capacity(vec, estimated_additions);
        if (samrena_vector_reserve_auto(vec, target_capacity) == SAMRENA_SUCCESS) {
            batch.reserved_capacity = target_capacity;
            batch.in_batch = true;
        }
    }
    
    return batch;
}

void samrena_vector_end_batch(SamrenaVectorBatch* batch) {
    // Optional: shrink to fit if significantly over-allocated
    if (batch && batch->in_batch) {
        batch->in_batch = false;
    }
}
```

## Testing Strategy

### Performance Benchmarks
```c
void benchmark_bulk_vs_individual() {
    const size_t count = 10000;
    int data[count];
    for (size_t i = 0; i < count; i++) data[i] = i;
    
    // Benchmark individual pushes
    clock_t start = clock();
    SamrenaVector* vec1 = samrena_vector_init_owned(sizeof(int), 10);
    for (size_t i = 0; i < count; i++) {
        samrena_vector_push_auto(vec1, &data[i]);
    }
    clock_t individual_time = clock() - start;
    
    // Benchmark bulk push
    start = clock();
    SamrenaVector* vec2 = samrena_vector_init_owned(sizeof(int), 10);
    samrena_vector_push_array(vec2, data, count);
    clock_t bulk_time = clock() - start;
    
    printf("Individual: %ld, Bulk: %ld, Speedup: %.2fx\n", 
           individual_time, bulk_time, (double)individual_time / bulk_time);
}
```

### Correctness Tests
- Element ordering after bulk operations
- Capacity management during bulk operations
- Error handling in edge cases
- Memory safety with overlapping operations

## Integration Notes
- Works with both owned and shared arena vectors
- Leverages existing capacity management system
- Uses established error handling patterns
- Maintains backward compatibility
# Task 09: Design Bulk Operations API

## Objective
Design efficient bulk operation APIs for Samvector that enable high-performance manipulation of multiple elements, including insertion, removal, and array operations.

## Requirements
- Efficient batch operations to minimize reallocation overhead
- Support for inserting/removing multiple elements at arbitrary positions
- Array-based operations for interoperability with C arrays
- Range-based operations for working with subsequences
- Consistent error handling with single-element operations

## Proposed API

### Bulk Insertion Operations
```c
// Push multiple elements from array
int samrena_vector_push_array(SamrenaVector* vec, const void* elements, size_t count);

// Insert multiple elements at specific position
int samrena_vector_insert_array(SamrenaVector* vec, size_t index, 
                               const void* elements, size_t count);

// Insert single element at specific position
int samrena_vector_insert(SamrenaVector* vec, size_t index, const void* element);

// Append contents of another vector
int samrena_vector_append_vector(SamrenaVector* dest, const SamrenaVector* src);

// Insert vector contents at specific position
int samrena_vector_insert_vector(SamrenaVector* dest, size_t index, 
                                const SamrenaVector* src);
```

### Bulk Removal Operations  
```c
// Remove element at specific index
int samrena_vector_remove(SamrenaVector* vec, size_t index);

// Remove multiple elements starting at index
int samrena_vector_remove_range(SamrenaVector* vec, size_t start, size_t count);

// Remove elements between start and end indices (exclusive end)
int samrena_vector_remove_slice(SamrenaVector* vec, size_t start, size_t end);

// Remove elements matching predicate
typedef bool (*SamrenaVectorPredicate)(const void* element, void* user_data);
size_t samrena_vector_remove_if(SamrenaVector* vec, SamrenaVectorPredicate pred, 
                               void* user_data);

// Fast removal by swapping with last element (order not preserved)
int samrena_vector_remove_swap(SamrenaVector* vec, size_t index);
```

### Array Interoperability
```c
// Copy vector contents to C array
int samrena_vector_to_array(const SamrenaVector* vec, void* array, 
                           size_t array_capacity);

// Create vector from C array
SamrenaVector* samrena_vector_from_array(const void* array, size_t count, 
                                        size_t element_size);

// Replace vector contents with array
int samrena_vector_assign_array(SamrenaVector* vec, const void* array, size_t count);

// Get slice of vector as array (returns pointer into vector data)
const void* samrena_vector_slice(const SamrenaVector* vec, size_t start, size_t count);
```

### Vector Manipulation
```c
// Create deep copy of vector
SamrenaVector* samrena_vector_copy(const SamrenaVector* src);

// Create copy with different arena
SamrenaVector* samrena_vector_copy_to_arena(Samrena* arena, const SamrenaVector* src);

// Resize vector to specific size (fill with zeros if growing)
int samrena_vector_resize(SamrenaVector* vec, size_t new_size);

// Resize vector with specific fill value
int samrena_vector_resize_with_value(SamrenaVector* vec, size_t new_size, 
                                    const void* fill_value);
```

## Design Considerations

### Performance Optimization Strategies
1. **Batch Memory Operations**: Use single allocation for multiple elements
2. **Memmove for Shifts**: Efficient element shifting during insert/remove
3. **Growth Prediction**: Smart capacity growth for bulk operations
4. **Cache-Friendly Access**: Contiguous memory operations where possible

### Memory Management
```c
// Pre-calculate required capacity for bulk operations
static size_t calculate_bulk_capacity(const SamrenaVector* vec, size_t additional) {
    size_t required = vec->size + additional;
    if (required <= vec->capacity) return vec->capacity;
    
    // Use exponential growth but consider bulk size
    size_t growth_by_factor = (size_t)(vec->capacity * vec->growth_factor);
    size_t growth_with_bulk = vec->capacity + additional;
    
    return (growth_by_factor > growth_with_bulk) ? growth_by_factor : growth_with_bulk;
}
```

### Element Shifting Strategy
```c
// Efficient element shifting for insertions
static void shift_elements_right(SamrenaVector* vec, size_t index, size_t count) {
    if (index >= vec->size) return;
    
    char* data = (char*)vec->data;
    size_t element_size = vec->element_size;
    
    // Source: starting at index
    char* src = data + (index * element_size);
    // Destination: index + count positions
    char* dst = data + ((index + count) * element_size);
    // Size: elements from index to end
    size_t move_size = (vec->size - index) * element_size;
    
    memmove(dst, src, move_size);
}
```

## Error Handling Strategy

### Validation Requirements
- Check vector and parameter validity
- Ensure sufficient capacity before operations
- Validate index bounds for all operations
- Handle edge cases (empty vectors, zero counts)

### Transaction-like Behavior
```c
// For operations that can partially fail, provide rollback capability
typedef struct {
    SamrenaVector* vector;
    size_t original_size;
    size_t original_capacity;
    void* original_data;
} SamrenaVectorTransaction;

// Begin transaction (saves state)
SamrenaVectorTransaction samrena_vector_begin_transaction(SamrenaVector* vec);

// Commit transaction (finalizes changes)
int samrena_vector_commit_transaction(SamrenaVectorTransaction* txn);

// Rollback transaction (restores original state)
void samrena_vector_rollback_transaction(SamrenaVectorTransaction* txn);
```

## API Usage Examples

### Bulk Insertion Example
```c
SamrenaVector* vec = samrena_vector_init_owned(sizeof(int), 10);

// Add multiple elements efficiently
int numbers[] = {1, 2, 3, 4, 5};
samrena_vector_push_array(vec, numbers, 5);

// Insert array in middle
int more[] = {10, 20, 30};
samrena_vector_insert_array(vec, 2, more, 3);
// Result: [1, 2, 10, 20, 30, 3, 4, 5]
```

### Bulk Removal Example
```c
// Remove range of elements
samrena_vector_remove_range(vec, 2, 3);  // Remove 3 elements starting at index 2
// Result: [1, 2, 4, 5]

// Remove elements matching condition
bool is_even(const void* element, void* user_data) {
    int value = *(const int*)element;
    return value % 2 == 0;
}

size_t removed = samrena_vector_remove_if(vec, is_even, NULL);
// Removes all even numbers
```

### Array Interoperability Example
```c
// Convert to regular array
int result_array[100];
samrena_vector_to_array(vec, result_array, 100);

// Create vector from existing array
int source[] = {100, 200, 300, 400};
SamrenaVector* vec2 = samrena_vector_from_array(source, 4, sizeof(int));
```

## Performance Characteristics

### Time Complexity Goals
- `push_array`: O(n) where n is number of elements added
- `insert_array`: O(m + n) where m is elements to shift, n is elements added
- `remove_range`: O(m) where m is elements to shift after removal
- `remove_if`: O(n) where n is total elements (single pass)

### Memory Efficiency
- Minimize reallocations through capacity prediction
- Use efficient memory movement (memmove vs element-by-element copy)
- Consider alignment requirements for performance

## Testing Requirements
- Bulk operation correctness
- Performance benchmarks vs individual operations
- Memory usage tracking
- Edge case handling (empty vectors, boundary conditions)
- Transaction rollback functionality

## Integration Notes
- Build upon existing capacity management
- Leverage error handling system
- Consider SIMD optimizations for large bulk operations
- Maintain compatibility with existing single-element API
# Samvector API Improvements Plan

## Overview

Samvector is a dynamic array implementation built on the samrena memory arena.
The current API provides basic functionality but lacks several features needed
for a production-ready vector library.

## Current API Analysis

### Existing Functions
- `samrena_vector_init()` - Initialize vector with element size and capacity
- `samrena_vector_push()` - Add element to end of vector
- `samrena_vector_pop()` - Remove and return last element
- `samrena_vector_resize()` - Manually resize vector capacity

### Current Limitations
- No direct element access methods
- Limited capacity management
- No bulk operations
- Missing search and iteration capabilities
- No error codes for better diagnostics
- No type safety mechanisms

## Proposed API Improvements

### 1. Element Access Functions
Currently missing direct element access methods:
- `samrena_vector_get(vec, index)` - Safe indexed access with bounds checking
- `samrena_vector_set(vec, index, element)` - Safe indexed assignment
- `samrena_vector_at(vec, index)` - Get pointer to element at index
- `samrena_vector_front(vec)` - Get first element
- `samrena_vector_back(vec)` - Get last element
- `samrena_vector_data(vec)` - Get raw data pointer

### 2. Capacity Management
- `samrena_vector_reserve(arena, vec, capacity)` - Reserve minimum capacity
- `samrena_vector_shrink_to_fit(arena, vec)` - Reduce capacity to match size
- `samrena_vector_clear(vec)` - Remove all elements without deallocating
- `samrena_vector_is_empty(vec)` - Check if vector is empty
- `samrena_vector_capacity(vec)` - Get current capacity
- `samrena_vector_size(vec)` - Get current size

### 3. Bulk Operations
- `samrena_vector_push_array(arena, vec, elements, count)` - Push multiple elements
- `samrena_vector_insert(arena, vec, index, element)` - Insert at specific position
- `samrena_vector_remove(vec, index)` - Remove element at index
- `samrena_vector_remove_range(vec, start, end)` - Remove range of elements
- `samrena_vector_copy(arena, src_vec)` - Create deep copy of vector

### 4. Search and Iteration
- `samrena_vector_find(vec, element, compare_fn)` - Find element using comparator
- `samrena_vector_contains(vec, element, compare_fn)` - Check if element exists
- `samrena_vector_foreach(vec, callback, user_data)` - Iterate with callback

### 5. Type Safety Improvements
Consider adding macro-based type-safe wrappers:
```c
#define SAMVECTOR_DEFINE_TYPE(T) \
    typedef struct { SamrenaVector base; } SamrenaVector_##T; \
    static inline T* samrena_vector_##T##_at(SamrenaVector_##T* vec, size_t idx) { ... }
```

### 6. Error Handling
- Return error codes instead of NULL for better diagnostics
- Add `samrena_vector_last_error()` for detailed error information
- Consider out-parameters for functions that can fail

### 7. Memory Control and Ownership
- **Arena Ownership**: Vectors can either own their arena or use an external one
  - Add `arena` and `owns_arena` fields to SamrenaVector struct
  - `samrena_vector_init_owned(element_size, capacity)` - Create vector with its own arena
  - `samrena_vector_init_with_arena(arena, element_size, capacity)` - Use external arena
  - `samrena_vector_destroy(vec)` - Cleanup vector (frees arena if owned)
- **Growth Strategies**:
  - `samrena_vector_init_with_allocator(arena, element_size, capacity, growth_factor)`
  - Allow custom growth strategies (1.5x, 2x, fixed increments)
  - `samrena_vector_trim(vec, new_size)` - Truncate vector to new size

#### Memory Safety Benefits
- **Flexibility**: Vectors can manage their own memory lifecycle or share an arena
- **Safety**: Clear ownership model prevents memory leaks
- **Performance**: Shared arenas allow efficient bulk allocations
- **API Design**: Functions no longer need arena parameter for owned vectors

#### Implementation Details
```c
typedef struct {
  uint64_t size;
  uint64_t element_size;
  uint64_t capacity;
  void *data;
  Samrena *arena;     // Arena used for allocations
  bool owns_arena;    // True if vector owns and should free the arena
} SamrenaVector;

// Create vector with its own arena
SamrenaVector* samrena_vector_init_owned(uint64_t element_size, uint64_t initial_capacity) {
    Samrena* arena = samrena_create(/* appropriate size */);
    SamrenaVector* vec = samrena_vector_init_with_arena(arena, element_size, initial_capacity);
    if (vec) {
        vec->owns_arena = true;
    }
    return vec;
}

// Operations no longer need arena parameter for owned vectors
void* samrena_vector_push_owned(SamrenaVector* vec, const void* element) {
    return samrena_vector_push(vec->arena, vec, element);
}
```

### 8. Utility Functions
- `samrena_vector_swap(vec, index1, index2)` - Swap two elements
- `samrena_vector_reverse(vec)` - Reverse element order
- `samrena_vector_sort(vec, compare_fn)` - Sort elements
- `samrena_vector_binary_search(vec, element, compare_fn)` - Binary search on sorted vector

### 9. Documentation and Safety
- Add comprehensive documentation for each function
- Document thread safety guarantees (currently not thread-safe)
- Add assertions/debug checks for common errors
- Consider adding `const` variants for read-only operations

## Implementation Priority

### Phase 1: Core Functionality (High Priority)
1. Element access functions
2. Capacity management functions
3. Error handling improvements

### Phase 2: Enhanced Operations (Medium Priority)
1. Bulk operations
2. Search and iteration
3. Basic utility functions

### Phase 3: Advanced Features (Lower Priority)
1. Type safety macros
2. Custom growth strategies
3. Performance optimizations

## Example Enhanced API Usage
```c
// Type-safe vector
SamrenaVector* vec = samrena_vector_init(arena, sizeof(int), 10);

// Reserve space for known size
samrena_vector_reserve(arena, vec, 1000);

// Bulk insert
int numbers[] = {1, 2, 3, 4, 5};
samrena_vector_push_array(arena, vec, numbers, 5);

// Safe access
int* value = samrena_vector_get(vec, 2);
if (value) printf("Value: %d\n", *value);

// Iteration
samrena_vector_foreach(vec, print_int, NULL);

// Clear without deallocation
samrena_vector_clear(vec);
```

## Benefits
- **Usability**: More intuitive API with common vector operations
- **Safety**: Bounds checking and better error handling
- **Performance**: Bulk operations and optimized paths
- **Flexibility**: Custom growth strategies and type safety
- **Compatibility**: Maintains arena-based memory model

## Backwards Compatibility

All existing functions will remain unchanged. New functions will be additions
to the API, ensuring existing code continues to work without modification.

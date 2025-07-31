# Task 03: Design Capacity Management API

## Objective
Design a comprehensive API for managing vector capacity, including reservation, shrinking, clearing, and querying operations that optimize memory usage and performance.

## Requirements
- Efficient capacity reservation to minimize reallocations
- Memory-conscious shrinking operations
- Clear without deallocation for reuse scenarios
- Query functions for size and capacity information
- Consistent error handling with arena operations

## Proposed API

### Capacity Control Functions
```c
// Reserve minimum capacity (may allocate more based on growth strategy)
// Returns: SAMRENA_SUCCESS or SAMRENA_ERROR_ALLOCATION_FAILED
int samrena_vector_reserve(SamrenaVector* vec, size_t min_capacity);

// Shrink capacity to match current size (may reallocate)
// Returns: SAMRENA_SUCCESS or SAMRENA_ERROR_ALLOCATION_FAILED
int samrena_vector_shrink_to_fit(SamrenaVector* vec);

// Set exact capacity (forces reallocation if different)
// Returns: SAMRENA_SUCCESS or SAMRENA_ERROR_ALLOCATION_FAILED
int samrena_vector_set_capacity(SamrenaVector* vec, size_t capacity);
```

### Content Management Functions
```c
// Remove all elements without changing capacity
void samrena_vector_clear(SamrenaVector* vec);

// Truncate vector to new size (no reallocation)
// Returns: SAMRENA_SUCCESS or SAMRENA_ERROR_OUT_OF_BOUNDS
int samrena_vector_truncate(SamrenaVector* vec, size_t new_size);

// Reset vector to initial state (clears and releases excess capacity)
int samrena_vector_reset(SamrenaVector* vec, size_t initial_capacity);
```

### Query Functions
```c
// Get current number of elements
size_t samrena_vector_size(const SamrenaVector* vec);

// Get current capacity
size_t samrena_vector_capacity(const SamrenaVector* vec);

// Check if vector is empty
bool samrena_vector_is_empty(const SamrenaVector* vec);

// Check if vector is at capacity
bool samrena_vector_is_full(const SamrenaVector* vec);

// Get available space before reallocation needed
size_t samrena_vector_available(const SamrenaVector* vec);
```

### Growth Control Functions
```c
// Set growth factor for automatic resizing
void samrena_vector_set_growth_factor(SamrenaVector* vec, float factor);

// Set minimum growth amount
void samrena_vector_set_min_growth(SamrenaVector* vec, size_t min_elements);

// Get memory usage statistics
typedef struct {
    size_t used_bytes;      // size * element_size
    size_t allocated_bytes; // capacity * element_size
    size_t wasted_bytes;    // allocated - used
    float utilization;      // used / allocated ratio
} SamrenaVectorStats;

SamrenaVectorStats samrena_vector_get_stats(const SamrenaVector* vec);
```

## Design Decisions

### Growth Strategy
- Default growth factor: 1.5x (balances memory and performance)
- Minimum growth: 8 elements (avoid tiny allocations)
- Allow custom growth configuration per vector

### Memory Management
- Reserve never shrinks capacity
- Shrink operations depend on arena capabilities
- Clear operation is O(1) - just resets size
- Consider arena constraints for all operations

### Arena Integration
- Vectors with owned arenas can reallocate freely
- Vectors using external arenas respect arena boundaries
- Shrink operations may be no-ops with certain arena types

### Error Handling
- Allocation failures return error codes
- Query functions are infallible (return 0 for NULL)
- Invalid parameters caught and reported

## Implementation Considerations

### Arena Compatibility
```c
// Need to handle different arena types
typedef struct {
    // ... existing fields ...
    float growth_factor;
    size_t min_growth;
    bool can_shrink;  // Arena supports shrinking
} SamrenaVector;
```

### Reallocation Strategy
1. Calculate new capacity based on growth strategy
2. Check if arena supports reallocation
3. Allocate new buffer if needed
4. Copy existing elements
5. Update vector metadata

### Performance Optimizations
- Inline simple query functions
- Avoid unnecessary copies during shrink
- Cache-align capacity boundaries
- Batch small growth operations

## Testing Requirements
- Capacity growth patterns
- Shrink behavior with different arena types
- Clear and truncate operations
- Memory usage tracking
- Growth strategy effectiveness

## Documentation
- Capacity vs size explanation
- Growth strategy guidelines
- Memory optimization tips
- Arena compatibility matrix